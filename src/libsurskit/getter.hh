#pragma once

template <typename Types>
using get_state = typename Types::State;

template <typename Types>
using get_model = typename Types::Model;

template <typename Types>
using get_search = typename Types::Search;

template <typename Types>
using get_node = typename Types::MatrixNode;
