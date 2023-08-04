#pragma once

template <typename Types>
concept IsNodeTypes =
    requires(
        typename Types::MatrixNode &matrix_node,
        typename Types::ChanceNode &chance_node,
        typename Types::Obs &obs,
        typename Types::Prob &prob,
        typename Types::State &state) {
        {
            matrix_node.access(0, 0)
        } -> std::same_as<typename Types::ChanceNode *>;
        {
            matrix_node.expand(state)
        } -> std::same_as<void>;
        {
            matrix_node.is_terminal()
        } -> std::same_as<bool>;
        {
            matrix_node.set_terminal(true)
        } -> std::same_as<void>;
        {
            matrix_node.set_terminal()
        } -> std::same_as<void>;
        {
            matrix_node.get_row_action(0)
        } -> std::same_as<typename Types::Action>;
        {
            matrix_node.get_col_action(0)
        } -> std::same_as<typename Types::Action>;
        {
            Types::MatrixNode::STORES_VALUE
        } -> std::convertible_to<bool>;
        {
            chance_node.access(obs)
        } -> std::same_as<typename Types::MatrixNode *>;
        {
            chance_node.row_idx
        } -> std::convertible_to<int>;
        {
            chance_node.col_idx
        } -> std::convertible_to<int>;
    } &&
    IsAlgorithmTypes<Types>;