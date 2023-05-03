#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>

void AdvancedSharedPtr();                   // Forward reference
void FileExample();                         // Forward reference
void WeakPtrDemo();                         // Forward reference

class Base {
public:
    virtual void speak() {
        std::cout << "I am a Base." << std::endl;
    }

    virtual ~Base() {
        std::cout << "Base destroyed." << std::endl;
    }
};

class Derived : public Base {
public:
    void speak() override {
        std::cout << "I am a Derived." << std::endl;
    }

    ~Derived() override {
        std::cout << "Derived destroyed." << std::endl;
    }
};

void custom_deleter(Base *ptr) {
    std::cout << "Custom deleter called." << std::endl;
    delete ptr;
}

int main() 
{
    // Creating a shared_ptr and initializing it with a new object
    std::shared_ptr<Base> sp1 = std::make_shared<Base>();
    sp1->speak();

    // Creating a shared_ptr with a custom deleter
    std::shared_ptr<Base> sp2(new Derived, custom_deleter);
    sp2->speak();

    // Creating a shared_ptr from another shared_ptr (copying)
    std::shared_ptr<Base> sp3 = sp1;
    sp3->speak();

    // Resetting a shared_ptr
    sp3.reset(new Derived);
    sp3->speak();

    // Checking the use_count of a shared_ptr
    std::cout << "sp1 use_count: " << sp1.use_count() << std::endl;

    // Using shared_ptr in a container
    std::vector<std::shared_ptr<Base>> container;
    container.push_back(sp1);
    container.push_back(sp2);
    container.push_back(sp3);

    for (const auto &item : container) {
        item->speak();
    }

    AdvancedSharedPtr();
    WeakPtrDemo();

    return 0;
}

//-------------------------------------------------------------------------
// Advanced shared_ptr examples
//-------------------------------------------------------------------------

void AdvancedSharedPtr() 
{
    // 1. Weak pointers
    std::shared_ptr<Base> sp1 = std::make_shared<Base>();
    std::weak_ptr<Base> wp1 = sp1;

    if (std::shared_ptr<Base> locked = wp1.lock()) {
        locked->speak();
    }

    sp1.reset();

    if (wp1.expired()) {
        std::cout << "The weak_ptr is expired." << std::endl;
    }

    // 2. Aliasing constructor
    std::shared_ptr<Base> sp2 = std::make_shared<Base>();
    std::shared_ptr<Base> sp2_alias(sp2, sp2.get());
    sp2_alias->speak();
    std::cout << "sp2 use_count: " << sp2.use_count() << std::endl;

    // 3. shared_ptr for arrays
    std::shared_ptr<Base[]> sp3(new Base[3]);
    sp3[0].speak();
    sp3[1].speak();
    sp3[2].speak();

    // 4. Atomic operations
    std::shared_ptr<Base> sp4 = std::make_shared<Base>();
    std::shared_ptr<Base> sp5;

    std::thread t1([&]() {
        std::atomic_store(&sp5, std::atomic_load(&sp4));
    });

    std::thread t2([&]() {
        std::atomic_store(&sp4, std::shared_ptr<Base>(nullptr));
    });

    t1.join();
    t2.join();

    if (sp5) {
        sp5->speak();
    }
}

//-------------------------------------------------------------------------
// Custom deleter example for FILE objects
//-------------------------------------------------------------------------

void file_deleter(FILE *file) {
    if (file) {
        std::cout << "Closing the file." << std::endl;
        fclose(file);
    }
}

void readFile(const char *filename) {
    // Create a shared_ptr with a custom deleter
    std::shared_ptr<FILE> file(fopen(filename, "r"), file_deleter);

    if (!file) {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }

    // Read the file
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file.get()) != nullptr) {
        std::cout << buffer;
    }

    // The file will be closed automatically by the custom deleter
}

void FileExample()
{
    readFile("main.cpp");
}

//-------------------------------------------------------------------------
// Weak pointer demo
//-------------------------------------------------------------------------

void WeakPtrDemo() 
{
    // Create a shared_ptr and initialize it with a new object
    std::shared_ptr<Base> sp1 = std::make_shared<Derived>();

    // Create a weak_ptr from the shared_ptr
    std::weak_ptr<Base> wp1(sp1);

    // Check if the weak_ptr is valid and lock it to obtain a shared_ptr
    if (auto locked = wp1.lock()) {
        std::cout << "Weak pointer is valid. Locked and obtained shared_ptr." << std::endl;
        locked->speak();
    } else {
        std::cout << "Weak pointer is not valid." << std::endl;
    }

    // Reset the shared_ptr, causing the object to be destroyed
    sp1.reset();

    // Check if the weak_ptr is still valid
    if (wp1.expired()) {
        std::cout << "Weak pointer is now expired." << std::endl;
    } else {
        std::cout << "Weak pointer is still valid." << std::endl;
    }
}