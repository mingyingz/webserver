#include "sql_connection_pool.h"
#include "../log/log.h"

std::shared_ptr<connection_pool> connection_pool::GetInstance() {
    static std::shared_ptr<connection_pool> p = std::make_shared<connection_pool>();
    return p;
}

connection_pool::connection_pool(): CurConn(0), FreeConn(0){}

connection_pool::~connection_pool(){
    DestroyPool();
}

void connection_pool::init(std::string url, std::string User, std::string PassWord, std::string DBName, int Port, unsigned int MaxConn){
    this->url = url;
    this->User = User;
    this->PassWord = PassWord;
    this->Port = Port;
    this->DatabaseName = DBName;
        
    std::unique_lock<std::mutex> lock(mutex);
    for (int i = 0; i < MaxConn; i++){
        MYSQL *con = nullptr;
        con = mysql_init(con);
        if (con == nullptr){
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, nullptr, 0);
        if (con == nullptr){
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        connList.push_back(con);
        ++FreeConn;
    }
    sem_init(&reserve, 0, FreeConn);
    this->MaxConn = FreeConn;
}

MYSQL *connection_pool::GetConnection(){
    MYSQL *con = nullptr;
    if (connList.empty()){
        return nullptr;
    }
    sem_wait(&reserve);
    {
        std::unique_lock<std::mutex> lock(mutex);
        con = connList.front();
        connList.pop_front();
        --FreeConn;
        ++CurConn;
    }
    return con;
}

bool connection_pool::ReleaseConnection(MYSQL *con){
    if (con == nullptr){
        return false;
    }
    std::unique_lock<std::mutex> lock(mutex);
    connList.push_back(con);
    ++FreeConn;
    --CurConn;
    sem_post(&reserve);
    return true;
}

void connection_pool::DestroyPool(){
    std::unique_lock<std::mutex> lock(mutex);
    if (connList.empty()){
        return;
    }
    for(auto &con : connList){
        mysql_close(con);
    }
    CurConn = 0;
    FreeConn = 0;
    connList.clear();
}

int connection_pool::GetFreeConn()
{
	return this->FreeConn;
}

connectionRAII::connectionRAII(MYSQL **con, std::shared_ptr<connection_pool> connPool){
    *con = connPool->GetConnection();
    conRAII = *con;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII(){
    poolRAII->ReleaseConnection(conRAII);
}