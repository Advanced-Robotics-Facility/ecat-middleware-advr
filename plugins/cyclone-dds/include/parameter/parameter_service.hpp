class ParameterServer
{
public:

    ParameterServer(
        dds::domain::DomainParticipant& participant);

private:

    ParameterRegistry registry_;

    ServiceServer<
        GetParameters_Request_,
        GetParameters_Response_> get_;

    ServiceServer<
        SetParameters_Request_,
        SetParameters_Response_> set_;

    ServiceServer<
        ListParameters_Request_,
        ListParameters_Response_> list_;
};