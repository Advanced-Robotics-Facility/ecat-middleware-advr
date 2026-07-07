class Node
{
public:

    Node(
        dds::domain::DomainParticipant& participant,
        std::string name);

    ParameterServer& parameters();

    template<typename T>
    Publisher<T> create_publisher(...);

    template<typename T>
    Subscriber<T> create_subscriber(...);

    template<typename Req, typename Rep>
    ServiceServer<Req,Rep> create_service(...);

    template<typename Req, typename Rep>
    ServiceClient<Req,Rep> create_client(...);

private:

    std::string name_;

    dds::domain::DomainParticipant& participant_;

    ParameterServer parameter_server_;
};