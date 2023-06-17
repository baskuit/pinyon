#pragma once

#include <vector>
#include <iterator>
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <ranges>
#include <string>

/*

TODO - make general

This is just a copy of the random tree generator. 
The idea is that we want a way to lazily apply a function/constructor to the output of cartesian_product

*/

template <typename ...Args>
struct RandomTreeGenerator
{

    using Tuple = std::tuple<size_t, size_t, size_t, double>;

    using Product = decltype(std::views::cartesian_product(depth_bound_vec, actions_vec, chance_action_vec, chance_threshold_vec));
    using It = std::ranges::iterator_t<Product>;

    Product view = std::views::cartesian_product(depth_bound_vec, actions_vec, chance_action_vec, chance_threshold_vec);

    RandomTreeGenerator()
    {
    }

    
    class Iterator : public It
    {
    public:
        RandomTreeGenerator *ptr;

        Iterator(const It &it, RandomTreeGenerator *ptr) : It{it}, ptr{ptr}
        {
        }

        Iterator &operator++()
        {
            if (trial == 0) {
                It::operator++();
            }
            ptr->seed = ptr->device.uniform_64();

            ++trial;
            trial %= ptr->trials;

            return (*this);
        }

        RandomTree operator*()
        {

            Tuple tuple = It::operator*();
            std::cout << std::get<0>(tuple) << ' ' << std::get<1>(tuple) << ' ' << std::get<2>(tuple) << ' ' << std::get<3>(tuple) << std::endl;

            return RandomTree{
                prng{ptr->seed},
                static_cast<int>(std::get<0>(tuple)),
                std::get<1>(tuple),
                std::get<1>(tuple),
                std::get<2>(tuple),
                std::get<3>(tuple)};
        }

        bool operator==(const Iterator &other) const
        {
            return static_cast<const It &>(*this) == static_cast<const It &>(other) && trial == other.trial;
        }
    };

    Iterator begin()
    {
        return Iterator(view.begin(), this);
    }

    Iterator end()
    {
        return Iterator(view.end(), this);
    }
};