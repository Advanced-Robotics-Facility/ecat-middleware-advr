class ParameterServer
{
public:

    template<typename T>
    void declare(
        const std::string& name,
        T default_value);

    template<typename T>
    void set(
        const std::string& name,
        T value);

    template<typename T>
    T get(
        const std::string& name) const;

    bool has(
        const std::string&) const;

    std::vector<Parameter> list() const;

private:

    std::unordered_map<
        std::string,
        Parameter_
    > parameters_;
};