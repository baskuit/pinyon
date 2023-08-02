#pragma once

template <typename MatrixNode>
concept IsMatrixNode = requires(MatrixNode matrix_node) {
    {
        matrix_node.is_terminal()
    } -> std::same_as<bool>;
};
