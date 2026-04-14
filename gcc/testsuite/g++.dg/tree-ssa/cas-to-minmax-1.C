// { dg-do compile { target c++11 } }
// { dg-options "-O2 -fdump-tree-cas-to-minmax" }

#include <atomic>
#include <algorithm>

template <typename T>
T atomic_fetch_min_explicit(std::atomic<T>* pv,
                            typename std::atomic<T>::value_type v,
                            std::memory_order m) noexcept {
    auto t = pv->load(std::memory_order_relaxed);
    while (!pv->compare_exchange_weak(t, std::min(v, t), m,
                                      std::memory_order_relaxed))
        ;
    return t;
}

template int atomic_fetch_min_explicit<int>(std::atomic<int>*, int,
                                            std::memory_order) noexcept;

// { dg-final { scan-tree-dump "ATOMIC_FETCH_MINMAX" "cas-to-minmax" } }
// { dg-final { scan-tree-dump "atomic MIN" "cas-to-minmax" } }
// { dg-final { scan-tree-dump-not "ATOMIC_COMPARE_EXCHANGE" "cas-to-minmax" } }
