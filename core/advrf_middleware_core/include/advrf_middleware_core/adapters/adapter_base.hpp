#pragma once

class AdapterBase {
    public:
        virtual ~AdapterBase() = default;
        virtual bool start() = 0;
        virtual void spin_once() = 0;
        virtual bool is_ok() const = 0;
};