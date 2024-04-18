#include <sys/socket.h>
#include <vector>
#include <unordered_map>
#include <memory>
// #include <netinet/in.h>

#include "./lock/locker.h"
#include "./threadpool/threadpool.h"
#include "./timer/lst_timer.h"
#include "./http/http_conn.h"
#include "./log/log.h"
#include "./CGImysql/sql_connection_pool.h"

const int MAX_FD = 65536;
const int TIMESLOT = 5;
const int MAX_EVENT_NUMBER = 10000;
const bool SYNLOG = false;

const TRIGMode connection_mode = ET;
const TRIGMode listen_mode = LT;

static int epollfd = 0;

int pipefd[2];

std::unordered_map<int, std::shared_ptr<http_conn>> users;

void addfd(int epollfd, int fd, bool one_shot, TRIGMode mode);
int setnonblocking(int fd);
std::unique_ptr<time_heap> timers;
std::atomic<int> http_conn::m_user_count(0);

void sig_handler(int sig){
    //触发信号不会改变errno，因此这里的errno就是触发信号前的errno
    int save_errno = errno;
    int msg = sig;
    printf("send msg = %d\n", msg);
    send(pipefd[1], (char *) &msg, 1, 0); //管道的0是读端，1是写端，将信号从管道的写端写入
    errno = save_errno;
    
}

void timer_update(int fd, int delay){
    LOG_INFO("%s", "adjust timer once");
    Log::get_instance()->flush();
    timers->adjust_timer(fd, time(NULL) + delay);
}

void do_read(std::unordered_map<int, std::shared_ptr<http_conn>> &users, std::weak_ptr<threadpool> pool, int sockfd){
    bool read_ret = users[sockfd]->read_once();
    if(read_ret){
        LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd]->get_address()->sin_addr));
        Log::get_instance()->flush();
        std::shared_ptr<threadpool> pool_ptr = pool.lock();
        if(pool_ptr == nullptr){
            assert(0);
            return;
        }
        // pool_ptr->append(std::bind(&http_conn::process, users[sockfd]));
        users[sockfd]->process();
        timer_update(sockfd, 3 * TIMESLOT);
    }
    else{
        timer_update(sockfd, 3 * TIMESLOT);
        timers->release_timer(sockfd);
    }
}

void do_write(std::unordered_map<int, std::shared_ptr<http_conn>> &users, std::weak_ptr<threadpool> pool, int sockfd){
    bool read_ret = users[sockfd]->write();
    if(read_ret){
        LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd]->get_address()->sin_addr));
        Log::get_instance()->flush();
        std::shared_ptr<threadpool> pool_ptr = pool.lock();
        if(pool_ptr == nullptr){
            assert(0);
            return;
        }
        // pool_ptr->append(std::bind(&http_conn::process, &users[sockfd]));
        // pool_ptr->append(std::bind(&http_conn::process, users[sockfd]));
        users[sockfd]->process();
        timer_update(sockfd, 3 * TIMESLOT);
    }
    else{
        timer_update(sockfd, 3 * TIMESLOT);
        timers->release_timer(sockfd);
    }
}

void addsig(int sig, void (handler) (int), bool restart = true){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if(restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void timer_handler(){
    timers->tick();
    alarm(TIMESLOT);

}

void show_error(int connfd, const char *info)
{
    printf("%s", info);
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

// 时间到期的回调函数
void cb_func(int fd){
    // epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    //assert( user_data );
    //users_timer.erase(user_data->sockfd);
    timers->del_timer(fd);
    users[fd]->close_conn();
    // users.erase(fd);
    LOG_INFO("close fd %d", fd);
    Log::get_instance()->flush();
}

int main(int argc, char *argv[]){
     if(!SYNLOG){
         Log::get_instance()->init("ServerLog", 2000, 800000, 8); //异步日志模型
     }
     else{
         Log::get_instance()->init("ServerLog", 2000, 800000, 0); //同步日志模型
     }

    if (argc <= 1){
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    int port = atoi(argv[1]);

    std::shared_ptr<connection_pool> connPool = connection_pool::GetInstance();
    connPool->init("localhost", "root", "123456", "webserver", 3306, 8);
    timers = std::make_unique<time_heap>();

    std::shared_ptr<threadpool> pool = std::make_shared<threadpool>(connPool, 16);

    users[0]->initmysql_result(connPool);

    // std::vector<epoll_event> events(MAX_EVENT_NUMBER);
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(10);
    assert(epollfd != -1);

    http_conn::get_m_epollfd() = epollfd;

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    //const char *ip = "172.30.67.47";
    //const char *ip = "127.0.0.1";
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    //inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_addr.s_addr = htonl(INADDR_ANY); //把s_addr设置为本机的IP地址
    address.sin_port = htons(port);

    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    int ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd, 5);
    assert(ret >= 0);

    addfd(epollfd, listenfd, false, listen_mode);



    // 创建双向管道。
    // socketpair的前三个参数与socket相同。
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);  //可以尝试PF_UNIX
    assert(ret != -1);

    setnonblocking(pipefd[1]);

    

    bool stop_server = false;

    addfd(epollfd, pipefd[0], false, connection_mode);

    addsig(SIGPIPE, SIG_IGN);
    addsig(SIGALRM, sig_handler, false);
    // addsig(SIGALRM, alarm_handler, true);
    addsig(SIGTERM, sig_handler, false);  //终止进程。kill命令默认发送发送的信号就是SIGTERM

    bool timeout = false;
    alarm(TIMESLOT);

    while(!stop_server){
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number < 0 && errno != EINTR){
            break;
        }
        // std:: cout << "========================================" << std::endl;
        for(int i = 0; i < number; i++){
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                if(listen_mode == LT){
                    int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
                    if(connfd < 0){
                        LOG_ERROR("%s:errno is:%d", "accept error", errno);
                        continue;
                    }
                    if (http_conn::get_m_user_count() >= MAX_FD){
                        show_error(connfd, "Internal server busy");
                        LOG_ERROR("%s", "Internal server busy");
                        continue;
                    }
                    if(users.find(connfd) == users.end())
                        users[connfd] = std::make_shared<http_conn>();
                    auto test = users[connfd];
                    if(test->m_sockfd != -1){
                        assert(0);
                        // sleep(10);
                    }
                    // assert(users[connfd]->m_sockfd == -1);
                    users[connfd]->init(connfd, client_address, connection_mode);
                    users[connfd]->pool = connPool;
                    timers->add_timer(connfd, 3 * TIMESLOT, cb_func);
                }
                else{
                    while (1){
                        int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
                        if (connfd < 0){
                            LOG_ERROR("%s:errno is:%d", "accept error", errno);
                            break;
                        }
                        if (http_conn::get_m_user_count() >= MAX_FD){
                            show_error(connfd, "Internal server busy");
                            LOG_ERROR("%s", "Internal server busy");
                            break;
                        } 
                        if(users.find(connfd) == users.end())
                            users[connfd] = std::make_shared<http_conn>();
                        users[connfd]->init(connfd, client_address, connection_mode);
                        users[connfd]->pool = connPool;
                        timers->add_timer(connfd, 3 * TIMESLOT, &cb_func);
                        continue;
                    }
                }
            }
            else if(events[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)){
                users[sockfd]->close_conn();
            }
            else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN)){
                int sig;
                char signals[1024];

                int ret = recv(pipefd[0], signals, sizeof(signals), 0);

                //从管道读端读出信号值，成功返回字节数，失败返回-1，如果对方关闭了连接，返回0
                //正常情况下，这里的ret返回值总是1，只有14和15两个ASCII码对应的字符
                if (ret == -1){
                    continue;
                }
                else if (ret == 0){
                    continue;
                }
                else{
                    for (int i = 0; i < ret; ++i){
                        switch (signals[i]){
                            case SIGALRM:{
                                timeout = true;
                                break;
                            }
                            case SIGTERM:{
                                stop_server = true;
                            }
                        }
                    }
                }
            }
            else if(events[i].events & EPOLLIN){
                timer_update(sockfd, 100 * TIMESLOT);
                pool->append(std::bind(do_read, users, pool, sockfd));
                // do_read(users, pool, sockfd);
            }
            else if(events[i].events & EPOLLOUT){
                timer_update(sockfd, 100 * TIMESLOT);
                pool->append(std::bind(do_write, users, pool, sockfd));
                // do_write(users, pool, sockfd);
            }

        }
        if (timeout){
            printf("timeout\n");
            timer_handler();
            timeout = false;
        }
    }
}
