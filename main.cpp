#include <iostream>
#include <map>
#include <list>

template<typename T, std::size_t capacity>
struct alloc {
    using value_type = T;
    std::list<void*> pool;
    std::size_t block_cap;
    std::size_t block_head = 0;

    alloc() noexcept : block_cap(capacity) {}

    template<class U>
    explicit alloc(const alloc<U, capacity> &other) noexcept : pool(other.pool),
                                                               block_cap(other.block_cap),
                                                               block_head(other.block_head) {}

    ~alloc() {
        for(auto const & p : pool){
            ::operator delete(p, capacity*sizeof(T));
        }
    }

/*    T *allocate(std::size_t n) {
        if(pool == nullptr) {
            pool = ::operator new(block_cap * sizeof(T));
            pool.push_back(pool);
        }
        if(block_head + n > block_cap) {
            //throw std::bad_alloc();
            pool = ::operator new(block_cap * sizeof(T));
            pool.push_back(pool);
            block_head = 0;
        }
    return reinterpret_cast<T *>(pool) + block_head++;
    }*/
    T *allocate(std::size_t n) {
        if(pool.empty()) {
            pool.push_back(::operator new(block_cap * sizeof(T)));
        }
        if(block_head + n > block_cap) {
            //throw std::bad_alloc();
            pool.push_back(::operator new(block_cap * sizeof(T)));
            block_head = 0;
        }
        return reinterpret_cast<T *>(pool.back()) + block_head++;
    }
    void deallocate (T* p, std::size_t n){}

    template <class Up, class... Args>
    void construct(Up* p, Args&&... args) {
        ::new ((void*)p) Up(std::forward<Args>(args)...);
    }
    void destroy(T* p) {
        p->~T();
    }

    template<class U, size_t cap = capacity>
    struct rebind {
        typedef alloc<U, cap> other;
    };

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
};

template<class T, class U, std::size_t cap>
constexpr bool operator==(const alloc<T, cap> &a1, const alloc<U,cap> &a2) noexcept {
    return a1.pool == a2.pool;
}

template<class T, class U, std::size_t cap>
constexpr bool operator!=(const alloc<T, cap> &a1, const alloc<U, cap> &a2) noexcept {
    return a1.pool != a2.pool;
}

template<typename T, typename Allocator = std::allocator<T>>
struct MyList{
    struct Node {
        using value_type = T;
        T val;
        Node* next;

        explicit Node(T _val) : val(_val), next(nullptr) {}
    };

    MyList() : front(nullptr), back(nullptr) {}

    bool is_empty() {
        return front == nullptr;
    }

    void push_back(T val) {
        Node* p = nodeAlloc.allocate(1);
        nodeAlloc.construct(p, val);

        if (is_empty()) {
            front = p;
            back = p;
            return;
        }
        back->next = p;
        back = p;
    }

    void pop_front() {
        if (is_empty()) return;
        Node* p = front;
        front = p->next;
        nodeAlloc.destroy(p);
        nodeAlloc.deallocate(p,sizeof(Node));
    }

    void pop_back() {
        if (is_empty()) return;
        if (front == back) {
            pop_front();
            return;
        }
        Node* p = front;
        while (p->next != back) p = p->next;
        p->next = nullptr;
        nodeAlloc.destroy(p);
        nodeAlloc.deallocate(p,sizeof(Node));
        back = p;
    }

    void print() {
        if (is_empty()) return;
        Node* p = front;
        while (p) {
            std::cout << p->val << " ";
            p = p->next;
        }
        std::cout << std::endl;
    }
private:

    Node* front;
    Node* back;
    typename std::allocator_traits<Allocator>::template rebind_alloc<Node> nodeAlloc;
};

int main() {
    std::map<int, int> map_std{{0, 1}};
    for (int i = 1; i < 10; ++i) {
        map_std[i] = map_std[i - 1] * i;
    }

    std::map<int, int, std::less<>, alloc<std::pair<const int, int>, 10>> map_cust{{0, 1}};
    for (int i = 1; i < 10; ++i) {
        map_cust[i] = map_cust[i - 1] * i;
    }

    for (const auto &[f, s]: map_cust) {
        std::cout << f << ' ' << s << std::endl;
    }

    MyList<int> list;
    for (int i = 0; i < 10; ++i) {
        list.push_back(i);
    }

    MyList<int, alloc<int, 10>> list_cust;
    for (int i = 0; i < 10; ++i) {
        list_cust.push_back(i);
    }
    list_cust.print();

    return 0;
}
