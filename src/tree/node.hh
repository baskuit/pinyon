#pragma once

template <typename Types>
concept IsNodeTypes =
    requires(
        typename Types::MatrixNode &matrix_node,
        typename Types::ChanceNode &chance_node,
        typename Types::Observation &obs,
        typename Types::Probability &prob,
        typename Types::State &state) {
        {
            matrix_node.access(0, 0)
        } -> std::same_as<typename Types::ChanceNode *>;
        // // {
        // //     matrix_node.expand(state)
        // // } -> std::convertible_to<void>;
        // {
        //     matrix_node.is_terminal()
        // } -> std::convertible_to<bool>;
        // {
        //     matrix_node.set_terminal(true)
        // } -> std::convertible_to<void>;
        // {
        //     matrix_node.set_terminal()
        // } -> std::convertible_to<void>;
        // {
        //     matrix_node.get_row_actions()
        // } -> std::same_as<typename Types::VectorReal>;
        // {
        //     matrix_node.get_col_actions()
        // } -> std::same_as<typename Types::VectorReal>;
        // {
        //     Types::MatrixNode::STORES_VALUE
        // } -> std::same_as<bool>;
        // {
        //     chance_node.access(obs)
        // } -> std::same_as<typename Types::MatrixNode *>;
        // {
        //     chance_node.row_idx
        // } -> std::convertible_to<int>;
        // {
        //     chance_node.col_idx
        // } -> std::convertible_to<int>;
    };// &&
    // IsAlgorithmTypes<Types>;