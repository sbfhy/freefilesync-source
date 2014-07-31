// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html       *
// * Copyright (C) Zenju (zenju AT gmx DOT de) - All Rights Reserved        *
// **************************************************************************

#ifndef FIXED_LIST_01238467085684139453534
#define FIXED_LIST_01238467085684139453534

#include <iterator>

namespace zen
{
//std::list(C++11) compatible class for inplace element construction supporting non-copyable/movable types
//may be replaced by C++11 std::list when available...or never...
template <class T>
class FixedList
{
    struct Node
    {
        Node() : next(nullptr), val() {}
        //no variadic templates on VC2010... :(
        template <class A>                                                       Node(A&& a)                                           : next(nullptr), val(std::forward<A>(a)) {}
        template <class A, class B>                                              Node(A&& a, B&& b)                                    : next(nullptr), val(std::forward<A>(a), std::forward<B>(b)) {}
        template <class A, class B, class C>                                     Node(A&& a, B&& b, C&& c)                             : next(nullptr), val(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c)) {}
        template <class A, class B, class C, class D>                            Node(A&& a, B&& b, C&& c, D&& d)                      : next(nullptr), val(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c), std::forward<D>(d)) {}
        template <class A, class B, class C, class D, class E>                   Node(A&& a, B&& b, C&& c, D&& d, E&& e)               : next(nullptr), val(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c), std::forward<D>(d), std::forward<E>(e)) {}
        template <class A, class B, class C, class D, class E, class F>          Node(A&& a, B&& b, C&& c, D&& d, E&& e, F&& f)        : next(nullptr), val(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c), std::forward<D>(d), std::forward<E>(e), std::forward<F>(f)) {}
        template <class A, class B, class C, class D, class E, class F, class G> Node(A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g) : next(nullptr), val(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c), std::forward<D>(d), std::forward<E>(e), std::forward<F>(f), std::forward<G>(g)) {}

        Node* next; //singly linked list is sufficient
        T val;
    };

public:
    FixedList() :
        firstInsert(nullptr),
        lastInsert(nullptr),
        sz(0) {}

    ~FixedList() { clear(); }

    template <class NodeT, class U>
    class ListIterator : public std::iterator<std::forward_iterator_tag, U>
    {
    public:
        ListIterator(NodeT* it = nullptr) : iter(it) {}
        ListIterator& operator++() { iter = iter->next; return *this; }
        inline friend bool operator==(const ListIterator& lhs, const ListIterator& rhs) { return lhs.iter == rhs.iter; }
        inline friend bool operator!=(const ListIterator& lhs, const ListIterator& rhs) { return !(lhs == rhs); }
        U& operator* () { return  iter->val; }
        U* operator->() { return &iter->val; }
    private:
        NodeT* iter;
    };

    typedef T value_type;
    typedef ListIterator<Node, T> iterator;
    typedef ListIterator<const Node, const T> const_iterator;
    typedef T& reference;
    typedef const T& const_reference;

    iterator begin() { return firstInsert; }
    iterator end()   { return iterator(); }

    const_iterator begin() const { return firstInsert; }
    const_iterator end  () const { return const_iterator(); }

    const_iterator cbegin() const { return firstInsert; }
    const_iterator cend  () const { return const_iterator(); }

    reference       front()       { return firstInsert->val; }
    const_reference front() const { return firstInsert->val; }

    reference&       back()       { return lastInsert->val; }
    const_reference& back() const { return lastInsert->val; }

    void emplace_back() { pushNode(new Node); }
    template <class A>                                                       void emplace_back(A&& a)                                           { pushNode(new Node(std::forward<A>(a))); }
    template <class A, class B>                                              void emplace_back(A&& a, B&& b)                                    { pushNode(new Node(std::forward<A>(a), std::forward<B>(b))); }
    template <class A, class B, class C>                                     void emplace_back(A&& a, B&& b, C&& c)                             { pushNode(new Node(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c))); }
    template <class A, class B, class C, class D>                            void emplace_back(A&& a, B&& b, C&& c, D&& d)                      { pushNode(new Node(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c), std::forward<D>(d))); }
    template <class A, class B, class C, class D, class E>                   void emplace_back(A&& a, B&& b, C&& c, D&& d, E&& e)               { pushNode(new Node(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c), std::forward<D>(d), std::forward<E>(e))); }
    template <class A, class B, class C, class D, class E, class F>          void emplace_back(A&& a, B&& b, C&& c, D&& d, E&& e, F&& f)        { pushNode(new Node(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c), std::forward<D>(d), std::forward<E>(e), std::forward<F>(f))); }
    template <class A, class B, class C, class D, class E, class F, class G> void emplace_back(A&& a, B&& b, C&& c, D&& d, E&& e, F&& f, G&& g) { pushNode(new Node(std::forward<A>(a), std::forward<B>(b), std::forward<C>(c), std::forward<D>(d), std::forward<E>(e), std::forward<F>(f), std::forward<G>(g))); }

    template <class Predicate>
    void remove_if(Predicate pred)
    {
        Node* prev = nullptr;
        Node* ptr = firstInsert;

        while (ptr)
            if (pred(ptr->val))
            {
                Node* next = ptr->next;
                deleteNode(ptr);
                ptr = next;

                if (prev)
                    prev->next = next;
                else
                    firstInsert = next;
                if (!next)
                    lastInsert = prev;
            }
            else
            {
                prev = ptr;
                ptr = ptr->next;
            }
    }

    void clear()
    {
        Node* ptr = firstInsert;
        while (ptr)
        {
            Node* next = ptr->next;
            deleteNode(ptr);
            ptr = next;
        }

        firstInsert = lastInsert = nullptr;
        assert(sz == 0);
    }

    bool empty() const { return firstInsert == nullptr; }
    size_t size() const { return sz; }

private:
    FixedList           (const FixedList&) = delete;
    FixedList& operator=(const FixedList&) = delete;

    void pushNode(Node* newNode) //throw()
    {
        if (lastInsert == nullptr)
        {
            assert(firstInsert == nullptr && sz == 0);
            firstInsert = lastInsert = newNode;
        }
        else
        {
            assert(lastInsert->next == nullptr);
            lastInsert->next = newNode;
            lastInsert = newNode;
        }
        ++sz;
    }

    void deleteNode(Node* oldNode)
    {
        assert(sz > 0);
        --sz;
        delete oldNode;
    }

    Node* firstInsert;
    Node* lastInsert; //point to last insertion; required by efficient emplace_back()
    size_t sz;
};
}

#endif //FIXED_LIST_01238467085684139453534
