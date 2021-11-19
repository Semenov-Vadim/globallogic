#include <iostream>
#include <thread>
#include <unistd.h> //to use usleep()
#include <mutex>

#define COUNT_PHILOSOPHERS 5

std::mutex g_display_mutex;


class Fork{
private:
    bool is_occupied;    //true if a philosopher holds this fork
    std::thread::id occupied_by;

public:
    Fork():is_occupied(false){
    }

    /**
     * @return     False if the fork is already occupied. True otherwise.
     */
    bool take_fork(std::thread::id ph){
        if (! is_occupied) {
            is_occupied = true;
            occupied_by = ph;
            return true;
        }
        return false;
    }

    void drop_fork(){
        is_occupied = false;
        occupied_by = std::thread::id(000000000000000);
    }

    std::thread::id get_occupied_by(){
        return occupied_by;
    }
};


class Philosopher{
private:
    Fork* nearest_forks[2];
public:
    Philosopher(){
    }

    Philosopher(Fork &f_fork, Fork &s_fork) {
        nearest_forks[0]= &f_fork;
        nearest_forks[1]= &s_fork;
    }

    bool take_forks(std::thread::id thread_num){
        if (!(*nearest_forks[0]).take_fork(thread_num)){
            return false;
        }
        if ( !(*nearest_forks[1]).take_fork(thread_num)) {
            (*nearest_forks[0]).drop_fork();
            return false;
        }
        return true;
    }

    void drop_forks(){
        (*nearest_forks[0]).drop_fork();
        (*nearest_forks[1]).drop_fork();
    }

    void start_thinking(){
        std::cout<< "thinking...\n";
    }

    bool am_i_holding_two_forks(std::thread::id thread_num){
        if(((*nearest_forks[0]).get_occupied_by() == thread_num) && ((*nearest_forks[1]).get_occupied_by() == thread_num)){
            return true;
        }
        else{
            return false;
        }
    }

    bool start_eating(){
        std::cout<< "eating...\n";
    }
};


void thread_thinking(Philosopher philosopher, std::thread::id this_id, unsigned int sleep_time){    
    g_display_mutex.lock();
    std::cout<< "philosopher " << this_id << " ";
    philosopher.start_thinking();
    g_display_mutex.unlock();
    
    usleep(sleep_time);
}


void thread_eating(Philosopher philosopher, std::thread::id this_id, unsigned int sleep_time){
    g_display_mutex.lock();
    std::cout<< "philosopher " << this_id << " ";
    philosopher.start_eating();
    g_display_mutex.unlock();

    usleep(sleep_time);

    g_display_mutex.lock();
    philosopher.drop_forks();
    g_display_mutex.unlock();
}

void eat_and_think(Philosopher philosopher) {
    std::thread::id this_id = std::this_thread::get_id();
    unsigned int sleep_time = 700000;

    for(;;){
        g_display_mutex.lock();
        if(philosopher.take_forks(this_id)){
            g_display_mutex.unlock();
            thread_eating(philosopher, this_id, sleep_time);
            /*if (philosopher.am_i_holding_two_forks(this_id) == false){
                thread_thinking(philosopher, this_id, sleep_time);
            }
            else{
                thread_eating(philosopher, this_id, sleep_time);
            }*/
        }
        else{
            g_display_mutex.unlock();
            thread_thinking(philosopher, this_id, sleep_time);
        }
    }
}


int main(int argc, char *argv[]) {
    Fork forks[COUNT_PHILOSOPHERS];
    Philosopher philosopher[COUNT_PHILOSOPHERS];

    for (int i=0; i<COUNT_PHILOSOPHERS; i++) {
        if (i == COUNT_PHILOSOPHERS - 1){
            philosopher[i] = Philosopher(forks[i], forks[0]);
        }
        else{
            philosopher[i] = Philosopher(forks[i], forks[i+1]);
        }
    }
    
    //creating and joining threads
    std::thread *threads = new std::thread[COUNT_PHILOSOPHERS];
    for(int i = 0; i < COUNT_PHILOSOPHERS; i++){
        threads[i] = std::thread(eat_and_think, philosopher[i]);
    }
    for(int i = 0; i < COUNT_PHILOSOPHERS; i++){
        threads[i].join();
    }
    
    return 0;
}