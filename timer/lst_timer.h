
#include <memory>
#include <functional>
#include <netinet/in.h>

class heap_timer;

class heap_timer{
public:
    heap_timer(int fd, int delay, const std::function<void(int)> &cb_func): fd(fd), cb_func(cb_func){
        expire = time( NULL ) + delay;
    }

public:
   int fd;
   time_t expire;
   std::function<void(int)> cb_func;
};

class time_heap{
public:
    time_heap() = default;
    ~time_heap() = default;

    void swap_timer(int id1, int id2){
        assert(id1 >= 0 && id1 < array.size());
        assert(id2 >= 0 && id2 < array.size());
        std::swap(array[id1], array[id2]);
        assert(fd2id.find(array[id1].fd) != fd2id.end());
        assert(fd2id[array[id1].fd] == id2);
        assert(fd2id.find(array[id2].fd) != fd2id.end());
        assert(fd2id[array[id2].fd] == id1);
        fd2id[array[id1].fd] = id1;
        fd2id[array[id2].fd] = id2;

    }
    void add_timer(int fd, int delay, const std::function<void(int)> &cb_func){
        if(fd2id.find(fd) == fd2id.end()){
            array.emplace_back(fd, delay, cb_func);
            int hole = array.size() - 1;
            fd2id[fd] = hole;
            while(hole > 0){
                int parent = (hole - 1) / 2;
                if(array[parent].expire > array[hole].expire){
                    swap_timer(parent, hole);
                }
                else{
                    break;
                }
                hole = parent;
            }
        }
        else{
            // assert(array[fd2id[fd]].cb_func == nullptr);
            array[fd2id[fd]].expire = time(NULL) + delay;
            array[fd2id[fd]].cb_func = cb_func;
            assert(array[fd2id[fd]].fd == fd);
            percolate_down(fd2id[fd]);
        }
        assert(array.size() == fd2id.size());
    }

    void del_timer(int fd){
        array[fd2id[fd]].cb_func = nullptr;
        assert(array.size() == fd2id.size());
    }

    // heap_timer top(){
    //     if(array.empty())
    //         return nullptr;
    //     return array[0];
    // }

    void pop_timer(){
        if(array.empty()){
            return;
        }
        swap_timer(0, array.size() - 1);
        fd2id.erase(array.back().fd);
        array.pop_back();
        percolate_down(0);
    }

    void adjust_timer(int fd, time_t timeout){
        assert(fd2id.find(fd) != fd2id.end());
        array[fd2id[fd]].expire = timeout;
        percolate_down(fd2id[fd]);
    }

    void release_timer(int fd){
        array[fd2id[fd]].cb_func(fd);
    }

    void tick(){
        time_t cur = time(NULL);
        while(!array.empty()){
            // if(!array[0]){
            //     break;
            // }
            if(array[0].expire > cur){
                break;
            }
            if(array[0].cb_func){
                // assert(array[0]->user_data.lock() != nullptr);
                array[0].cb_func(array[0].fd);
            }
            pop_timer();
        }
        std::cout << array.size() << std::endl;
    }

private:

    void percolate_down(int hole){
        while(2 * hole + 1 < array.size()){
            int child = 2 * hole + 1;
            if(child + 1 < array.size() && array[child].expire > array[child + 1].expire){
                child++;
            }
            if(array[hole].expire > array[child].expire){
                swap_timer(hole, child);
            }
            else{
                break;
            }
            hole = child;
        }
    }
    std::vector<heap_timer> array;
    std::unordered_map<int, int> fd2id;
};
