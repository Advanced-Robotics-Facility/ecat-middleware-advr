#pragma once

enum class ParameterResult
{
    Success,
    AlreadyDeclared,
    NotFound,
    ReadOnly,
    ValidationFailed
};