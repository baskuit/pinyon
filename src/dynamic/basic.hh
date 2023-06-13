#include <memory>
#include <vector>
#include <string>
#include <any>
#include <unordered_map>

#define SEARCH_PARAMS(State, Model, BanditAlgorithm, TreeBandit)                                            \
    template <                                                                                              \
        class State,                                                                                        \
        template <                                                                                          \
            class>                                                                                          \
        class Model,                                                                                        \
        template <                                                                                          \
            class,                                                                                          \
            template <class, class, template <class> class, template <class> class, template <class> class> \
            class,                                                                                          \
            template <class> class,                                                                         \
            template <class> class>                                                                         \
        class BanditAlgorithm,                                                                              \
        template <                                                                                          \
            class,                                                                                          \
            class,                                                                                          \
            template <class> class,                                                                         \
            template <class> class,                                                                         \
            template <class> class>                                                                         \
        class TreeBandit>

namespace W
{

    template <typename _State>
    struct StateWrapper;
    template <typename _Model>
    struct ModelWrapper;
    template <typename E>
    struct SearchWrapper;

    struct State
    {
        State() {}
        virtual void get_actions() = 0;
        virtual void get_actions(size_t &rows, size_t &cols) = 0;
        virtual void apply_actions(int row_idx, int col_idx) = 0;
        virtual bool is_terminal() = 0;
        virtual double row_payoff() = 0;
        virtual double col_payoff() = 0;

        template <typename T>
        std::shared_ptr<T> deref () {
            StateWrapper<T> *self = dynamic_cast<StateWrapper<T>*>(this);
            return self->ptr;
        }
    };
    struct Model
    {
        struct Output
        {
            std::vector<double> row_policy, col_policy;
            double row_value, col_value;
        };
        virtual Output get_inference(State &state) = 0;
    };
    struct Search // = Algorithm + Tree
    {
        struct TreeData
        {
            // num nodes, branch factor etc
            // dont specialize into functions because its calculated by traversing tree
            // thus why not calculate everything. faster if we every need more than one piece of info..
        };

        TreeData tree_data;
        // expensive to calculate, so store as member
    };

    template <typename _State>
    struct StateWrapper : State
    {
        std::shared_ptr<_State> ptr;
        StateWrapper(const _State &state) : ptr{std::make_shared<_State>(state)} {}
        StateWrapper(const _State &&state) : ptr{std::make_shared<_State>(state)} {}
        StateWrapper(const StateWrapper &state_wrapper) : ptr{state_wrapper.ptr} {}
        template <typename... Args>
        StateWrapper(Args... args) : ptr(std::make_shared<_State>(args...)) {}

        bool is_constant_sum()
        {
            return typename _State::Types::Value::IS_CONSTANT_SUM();
        };
        double payoff_sum()
        {
            return typename _State::Types::Value::PAYOFF_SUM();
        };

        void get_actions()
        {
            ptr->get_actions();
        }
        void get_actions(size_t &rows, size_t &cols)
        {
            ptr->get_actions();
            rows = ptr->row_actions.size();
            rows = ptr->col_actions.size();
        };
        void apply_actions(int row_idx, int col_idx)
        {
            if (row_idx < ptr->row_actions.size() && col_idx < ptr->col_actions.size())
            {
                ptr->apply_actions(ptr->row_actions[row_idx], ptr->col_actions[col_idx]);
            }
        };
        bool is_terminal()
        {
            return ptr->is_terminal;
        };
        double row_payoff()
        {
            return ptr->payoff.get_row_value();
        };
        double col_payoff()
        {
            return ptr->payoff.get_col_value();
        };
        bool is_solved()
        {
            return false;
        };
        std::vector<double> row_strategy()
        {
            return std::vector<double>{};
        };
        std::vector<double> col_strategy()
        {
            return std::vector<double>{};
        };
    };

    template <typename _Model>
    struct ModelWrapper : Model
    {
        std::shared_ptr<_Model> ptr;
        typename _Model::ModelOutput output{};
        ModelWrapper(const _Model &model) : ptr{std::make_shared<_Model>(model)} {}
        ModelWrapper(const _Model &&model) : ptr{std::make_shared<_Model>(model)} {}
        ModelWrapper(const ModelWrapper &model_wrapper) : ptr{model_wrapper.ptr} {}
        template <typename... Args>
        ModelWrapper(Args... args) : ptr(std::make_shared<_Model>(args...)) {}

        Output get_inference(State &state)
        {
            auto raw_state = *state.deref<typename _Model::Types::State>();
            ptr->get_inference(raw_state, output);
            return Output{std::vector<double>{}, std::vector<double>{}, 0, 0};
        };
    };

    // template <typename... Args>
    // struct SearchWrapper;

    template <typename _Algorithm>
    struct SearchWrapper : Search
    {
        std::shared_ptr<_Algorithm> ptr;
        SearchWrapper(const _Algorithm &state) : ptr{std::make_shared<_Algorithm>(state)} {}
        SearchWrapper(const _Algorithm &&state) : ptr{std::make_shared<_Algorithm>(state)} {}
        SearchWrapper(const SearchWrapper &state_wrapper) : ptr{std::make_shared<_Algorithm>(*state_wrapper.ptr)} {}
        template <typename... Args>
        SearchWrapper(Args... args) : ptr(std::make_shared<_Algorithm>(args...)) {}
        // SearchWrapper<Battle,

        void run(size_t iterations, State &state, Model &model){};
        void get_empirical_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy){};
        void get_refined_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy){};
        void get_emprical_value(double &row_value, double &col_value){};
        double get_exploitability() { return 0; }; // aka expected regret because we use empirical strategies
    };

    // template <typename _Model, template <typename> typename __Algorithm>
    // struct SearchWrapper<__Algorithm<_Model>>
    // {
    //     using _Algorithm = __Algorithm<_Model>;
    //     std::shared_ptr<_Algorithm> ptr;
    //     SearchWrapper(const _Algorithm &state) : ptr{std::make_shared<_Algorithm>(state)} {}
    //     SearchWrapper(const _Algorithm &&state) : ptr{std::make_shared<_Algorithm>(state)} {}
    //     SearchWrapper(const SearchWrapper &state_wrapper) : ptr{std::make_shared<_Algorithm>(*state_wrapper.ptr)} {}
    //     template <typename... Args>
    //     SearchWrapper(Args... args) : ptr(std::make_shared<_Algorithm>(args...)) {}
    //     // SearchWrapper<Battle,

    //     void run(size_t iterations, State &state, Model &model){};
    //     void get_empirical_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy){};
    //     void get_refined_strategies(std::vector<double> &row_strategy, std::vector<double> &col_strategy){};
    //     void get_emprical_value(double &row_value, double &col_value){};
    //     double get_exploitability() { return 0; }; // aka expected regret because we use empirical strategies
    // };

    template <typename _MatrixNode>
    struct MatrixNodeWrapper
    {
        std::shared_ptr<_MatrixNode> ptr;
        std::unordered_map<std::string, std::any> stats() = 0;
        void get_metadata() = 0;
    };
};

// State, Model, Search, update_model(state = inference...)?