#pragma once

class AdapterBase {
    public:
        virtual ~AdapterBase() = default;
        virtual void spin_once() = 0;
};