#include "LikeMagic/Interprocess/RequestBroker.hpp"

namespace LM {

class RPC : public RequestBroker
{
private:
    boost::interprocess::shared_memory_object* shm;
    boost::interprocess::mapped_region* region;
    SharedMemoryFormat* data;
    bool is_first;
    boost::unordered_map<int, CallReturn> cached_rvalues;

    int invocation_counter;
    ProcessControlStructure* pcs;
    ProcessControlStructure* other_pcs;

    int execute(int method, int arg);
    std::string get_state_name(ProcessState s) const;

    SharedArgTransporter transporter;

public:
    RPC(AbstractTypeSystem& type_system_, bool is_first_);
    ~RPC();
    CallReturn listen(int invocation_id, bool wants_rvalue);
    CallReturn call(int object_handle, int method, int arg);
    int call_int(int method, int arg);
    void scan() const;
};

}}
