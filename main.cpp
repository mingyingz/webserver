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
#include <atomic>

const int MAX_FD = 65536;
const int TIMESLOT = 5;
const int MAX_EVENT_NUMBER = 10000;
const bool SYNLOG = false;

int count = 0;
const TRIGMode connection_mode = ET;
const TRIGMode listen_mode = LT;
std::atomic<long long> time_read(0);
std::atomic<long long> time_process(0);
std::atomic<long long> time_write(0);
std::atomic<long long> time_epoll(0);
std::atomic<long long> time_add(0);
std::atomic<long long> time_wait(0);
std::atomic<long long> time_timer(0);
std::atomic<long long> time_loop(0);
std::atomic<long long> time_listen(0);
std::atomic<long long> time_rw(0);
std::atomic<long long> time_close(0);


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

long long get_time(){
    timeval currentTime;
    gettimeofday(&currentTime, nullptr);
    return (long long)currentTime.tv_sec * 1000000 + currentTime.tv_usec;

}

void timer_update(int fd, int delay){
    long long t1 = get_time();
    LOG_INFO("%s", "adjust timer once");
    Log::get_instance()->flush();
    timers->adjust_timer(fd, time(NULL) + delay);
    long long t2 = get_time();
    time_timer.fetch_add(t2 - t1);
}


void do_read(std::unordered_map<int, std::shared_ptr<http_conn>> &users, std::weak_ptr<threadpool> pool, int sockfd){
    long long t1 = get_time();
    bool read_ret = users[sockfd]->read_once();
    long long t2 = get_time();
    time_read.fetch_add(t2 - t1);
    if(read_ret){
        LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd]->get_address()->sin_addr));
        Log::get_instance()->flush();
        std::shared_ptr<threadpool> pool_ptr = pool.lock();
        if(pool_ptr == nullptr){
            assert(0);
            return;
        }
        //pool_ptr->append(std::bind(&http_conn::process, ptr));
        //pool_ptr->append([users, sockfd](){users[sockfd]->process();});
	std::shared_ptr<http_conn> ptr = users[sockfd];
	assert(ptr);
	//std::cout << users.size() << std::endl;
	auto func = [ptr]() {ptr->process();};
	pool_ptr->append(func);
	//users[sockfd]->process();
        // timer_update(sockfd, 3 * TIMESLOT);
    }
    else{
        // std::cout << "rrrrrrr: " << sockfd << std::endl;
        // timer_update(sockfd, 10 * TIMESLOT);
        // printf("read fail\n");
        // printf("read fail\n");
        timers->release_timer(sockfd);
    }
}

void do_write(std::unordered_map<int, std::shared_ptr<http_conn>> &users, std::weak_ptr<threadpool> pool, int sockfd){
    long long t1 = get_time();
    std::shared_ptr<http_conn> ptr = users[sockfd];
    bool read_ret = ptr->write();
    long long t2 = get_time();
    time_write.fetch_add(t2 - t1);
    if(read_ret){
        LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd]->get_address()->sin_addr));
        Log::get_instance()->flush();
        // std::shared_ptr<threadpool> pool_ptr = pool.lock();
        // if(pool_ptr == nullptr){
        //     assert(0);
        //     return;
        // }
        // pool_ptr->append(std::bind(&http_conn::process, &users[sockfd]));
        // pool_ptr->append(std::bind(&http_conn::process, users[sockfd]));
        // users[sockfd]->process();
        // timer_update(sockfd, 3 * TIMESLOT);
    }
    else{
        // timer_update(sockfd, 10 * TIMESLOT);
        
        // std::cout << "wwwwwwwwww: " << sockfd << std::endl;
        // printf("write fail\n");
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
    long long t1 = get_time();
    timers->tick();
    alarm(TIMESLOT);
    long long t2 = get_time();
    time_timer.fetch_add(t2 - t1);

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
    //  if(!SYNLOG){
    //      Log::get_instance()->init("ServerLog", 2000, 800000, 8); //异步日志模型
    //  }
    //  else{
    //      Log::get_instance()->init("ServerLog", 2000, 800000, 0); //同步日志模型
    //  }

    int ret;
    //std::cout << photon::INIT_EVENT_DEFAULT << " " <<  photon::INIT_IO_NONE << std::endl;
    //int ret = photon::init();
    //if (ret != 0) {
    //    return -1;
    //}
    //DEFER(photon::fini());

    if (argc <= 1){
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    int port = atoi(argv[1]);

    std::shared_ptr<connection_pool> connPool = connection_pool::GetInstance();
    connPool->init("localhost", "root", "123456", "webserver", 3306, 8);
    timers = std::make_unique<time_heap>();

    std::shared_ptr<threadpool> pool = std::make_shared<threadpool>(connPool, 15);

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
    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
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
        long long t1 = get_time();
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        long long t2 = get_time();
        time_epoll.fetch_add(t2 - t1);
	count++;
        if(number < 0 && errno != EINTR){
            break;
        }
        // std:: cout << "========================================" << std::endl;
        t1 = get_time();
        for(int i = 0; i < number; i++){
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
        	long long t1 = get_time();
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
        	long long t2 = get_time();
        	time_listen.fetch_add(t2 - t1);
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
                timer_update(sockfd, 3 * TIMESLOT);
                long long t1 = get_time();
                // auto func = std::bind(do_read, users, pool, sockfd);
                // pool->append(func);
                do_read(users, pool, sockfd);
                long long t2 = get_time();
                time_rw.fetch_add(t2 - t1);
            }
            else if(events[i].events & EPOLLOUT){
                timer_update(sockfd, 3 * TIMESLOT);
    		    long long t1 = get_time();
                //auto func = std::bind(do_write, users, pool, sockfd);
                // auto func = [=, &users](){do_write(users, pool, sockfd);};
                long long t2 = get_time();
                time_close.fetch_add(t2 - t1);
                t1 = get_time();
                // pool->append(func);
                do_write(users, pool, sockfd);
                t2 = get_time();
                time_rw.fetch_add(t2 - t1);
            }
            else{
                assert(0);
            }

        }
        t2 = get_time();
        time_loop.fetch_add(t2 - t1);
        if (timeout){
            printf("timeout\n");
	    std::cout << "read time: " << time_read << std::endl;
            std::cout << "write time: " << time_write << std::endl;
            std::cout << "process time: " << time_process << std::endl;
            std::cout << "epoll time: " << time_epoll << std::endl;
            std::cout << "add time: " << time_add << std::endl;
            std::cout << "wait time: " << time_wait << std::endl;
            std::cout << "timer time: " << time_timer << std::endl;
            std::cout << "loop time: " << time_loop << std::endl;
            std::cout << "listen time: " << time_listen << std::endl;
            std::cout << "rw time: " << time_rw << std::endl;
            std::cout << "close time: " << time_close << std::endl;
            std::cout << "count: " << count << std::endl;
	    count = 0;
            time_read = 0;
            time_process = 0;
            time_wait = 0;
            time_add = 0;
            time_epoll = 0;
            time_write = 0;
            time_timer = 0;
            time_loop = 0;
            time_listen = 0;
            time_rw = 0;
            time_close = 0;
            timer_handler();
            timeout = false;
        }
    }
}
