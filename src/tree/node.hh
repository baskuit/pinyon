#pragma once

template <class _Algorithm>
class AbstractNode
{
public:
    struct Types : _Algorithm::Types
    {
        using Algorithm = _Algorithm;
    };
};