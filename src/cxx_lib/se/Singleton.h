/*
 * filename：Singleton
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Singleton template
 */
#ifndef IMMEDIATERENDERERVK_SINGLETON_H
#define IMMEDIATERENDERERVK_SINGLETON_H
namespace se {
    /**
     *@author Chilliziehen scream303677483@gmail.com
     *@brief Singleton template class, multi-thread safe in C++11 and later versions.
     *@note
     *      Usage:
     *      class MyClass : public Singleton<MyClass> {
     *          friend class Singleton<MyClass>;
     *      private:
     *          MyClass() {}
     *      public:
     *          void myMethod() {}
     *      };
     */
    template<class T>
    class Singleton {
    protected:
        Singleton() = default;
        ~Singleton() = default;
    public:
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

        /**
         *@note C++11 and later versions guarantee that
         *      the static local variable is initialized in a thread-safe manner.
         */
        static T& getInstance() {
            static T instance;// Construction here. Guaranteed to be destroyed.
            return instance;
        }
    };
}

#endif //IMMEDIATERENDERERVK_SINGLETON_H