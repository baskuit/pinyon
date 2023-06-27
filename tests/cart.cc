// #include <tuple>
// #include <vector>
// #include <type_traits>
// #include <ranges>
// #include <functional>

// template <typename... Containers>
// struct CartesianProductGenerator
// {
//     std::tuple<Containers...> containers;

//     using Tuple = std::tuple<typename Containers::value_type...>;
//     using Product = decltype(std::views::cartesian_product(std::declval<Containers>()...));
//     using It = std::ranges::iterator_t<Product>;

//     Product view = std::views::cartesian_product(containers);

//     CartesianProductGenerator(Containers&&... args)
//         : containers(std::forward<Containers>(args)...)
//     {
//     }

//     class Iterator : public It
//     {
//     public:
//         CartesianProductGenerator *ptr;

//         Iterator(const It &it, CartesianProductGenerator *ptr) : It(it), ptr(ptr) {}

//         Iterator &operator++()
//         {
//             It::operator++();
//             return (*this);
//         }

//         auto operator*()
//         {
//             Tuple tuple = It::operator*();
//             return std::apply(ptr->function, tuple);
//         }

//         bool operator==(const Iterator &other) const
//         {
//             return static_cast<const It &>(*this) == static_cast<const It &>(other);
//         }
//     };

//     Iterator begin()
//     {
//         return Iterator(view.begin(), this);
//     }

//     Iterator end()
//     {
//         return Iterator(view.end(), this);
//     }

//     template <typename F>
//     void setFunction(F &&function)
//     {
//         this->function = std::forward<F>(function);
//     }

// private:
//     std::function<Tuple(Containers...)> function;
// };

int main(

)
{
    // std::vector<int> a = {0, 1};
    // std::vector<int> b = {0, 1};
    // CartesianProductGenerator<std::vector<int>, std::vector<int>> gen(std::move(a), std::move(b));
}