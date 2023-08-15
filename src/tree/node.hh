#pragma once

template <typename Types>
concept IsNodeTypes =
    requires(
        typename Types::MatrixNode &matrix_node,
        const typename Types::MatrixNode &const_matrix_node,
        typename Types::ChanceNode &chance_node,
        const typename Types::ChanceNode &const_chance_node,
        typename Types::Obs &obs,
        typename Types::Prob &prob,
        typename Types::State &state) {
        {
            matrix_node.stats
        } -> std::same_as<typename Types::MatrixStats &>;
        {
            matrix_node.access(0, 0)
        } -> std::same_as<typename Types::ChanceNode *>;
        {
            const_matrix_node.access(0, 0)
        } -> std::same_as<const typename Types::ChanceNode *>;
        {
            matrix_node.expand(state)
        } -> std::same_as<void>;
        {
            const_matrix_node.is_terminal()
        } -> std::same_as<bool>;
        {
            matrix_node.set_terminal(true)
        } -> std::same_as<void>;
        {
            matrix_node.set_terminal()
        } -> std::same_as<void>;
        {
            const_matrix_node.get_row_actions()
        } -> std::same_as<typename Types::VectorAction>;
        {
            const_matrix_node.get_col_actions()
        } -> std::same_as<typename Types::VectorAction>;
        {
            const_matrix_node.get_row_action(0)
        } -> std::same_as<typename Types::Action>;
        {
            const_matrix_node.get_col_action(0)
        } -> std::same_as<typename Types::Action>;
        {
            Types::MatrixNode::STORES_VALUE
        } -> std::same_as<const bool &>;

        {
            chance_node.stats
        } -> std::same_as<typename Types::ChanceStats &>;
        {
            chance_node.access(obs)
        } -> std::same_as<typename Types::MatrixNode *>;
        {
            const_chance_node.access(obs)
        } -> std::same_as<const typename Types::MatrixNode *>;
        {
            chance_node.row_idx
        } -> std::same_as<int &>;
        {
            chance_node.col_idx
        } -> std::same_as<int &>;
    };
