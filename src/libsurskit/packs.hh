#pragma once

template <template <typename> typename... Template>
struct TemplatePack
{
};

template <typename... Type>
struct TypePack
{
};
