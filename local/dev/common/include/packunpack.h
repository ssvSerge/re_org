#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <initializer_list>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

// used in testing
#define USE_PACKET_CALLBACK

namespace packunpack
{
    // How to use:
    //   On the client side, do what's done in the HBSEClient::Execute__test().
    //   Basically, only
    // dispatcher::call_on_server needs to be executed, with correctly described
    // parameters.
    //   On the server side, do what's done in HBSETransceiver::InitHandlers() - for
    //   every function,
    // dispatcher::add_handler needs to be called with parameters specified in the
    // same order and flavour as on the client side. Dispatcher needs to call
    // mydispatcher::dispatch.

    // allows to serialize types with trivial constructors such as KeyInfoStructure
    template <typename TYPE>
    constexpr bool is_pod_v = std::is_pod_v<TYPE> ||
        (/*std::is_trivially_copyable_v<TYPE> &&*/ std::is_standard_layout_v<TYPE>);

    class packunpack_exception : public std::exception
    {
    };

    // a stream from which parameters are deserialized
    class input_params_stream
    {
    public:
        input_params_stream(const void *begin, const void *end) :
            stream((const uint8_t *)begin), pos(stream), end((const uint8_t *)end)
        {
            assert(!!begin == !!end); // both may be null
            assert(begin <= end);
        }
        template <typename TYPE>
        input_params_stream(const std::vector<TYPE> &buffer) :
            input_params_stream(buffer.data(), buffer.data() + buffer.size())
        {
        }

        const uint8_t *gimme(size_t size, bool exception_on_error = true)
        {
            if (!size)
                return nullptr;
            // assert(stream);
            if (!stream)
            {
                throw packunpack_exception(); // will fire when server crashes
                // return nullptr;
            }

            const uint8_t *newpos = pos + size;
            // assert(newpos <= end);
            if (newpos > end)
            {
                if (exception_on_error)
                    throw packunpack_exception(); // will fire when there is not enough data
                                                  // on stream for defined parameters
                else
                    return nullptr;
            }
            return std::exchange(pos, newpos);
        }
        template <typename TYPE>
        const TYPE *gimme(bool exception_on_error = true)
        {
            return (const TYPE *)gimme(sizeof(TYPE), exception_on_error);
        }

        void rewind() { pos = stream; }
        bool empty() const { return pos == end; }

    protected:
        const uint8_t *const stream;
        const uint8_t *pos;
        const uint8_t *const end;
    };

    // a stream into which parameters are serialized
    class output_params_stream
    {
    public:
        output_params_stream(std::function<void(uint8_t **)> delete_buffer = nullptr) :
            delete_buffer(std::move(delete_buffer))
        {
        }

        void reserve(std::initializer_list<size_t> params_sizes)
        {
            // used on the client size
            storage.reserve(std::accumulate(params_sizes.begin(), params_sizes.end(), size_t(0)));
        }
        template <typename PARAMS>
        void reserve(const PARAMS &params)
        {
            // used on the server side
            size_t size = 0;
            std::apply([&size](const auto &...p) { ((size += p.get_out_size()), ...); }, params);
            storage.reserve(size);
        }

        void append(const void *begin, const void *end)
        {
            storage.insert(storage.end(), (const uint8_t *)begin, (const uint8_t *)end);
        }
        void append(const void *begin, size_t length) { append(begin, (const uint8_t *)begin + length); }
        template <typename TYPE>
        void append(const TYPE &data)
        {
            static_assert(is_pod_v<TYPE>);
            append(&data, sizeof(data));
        }

        std::vector<uint8_t> steal_storage() { return std::move(storage); }
        const std::vector<uint8_t> &get_storage() const { return storage; }
        std::vector<uint8_t> &get_storage() { return storage; }
        const std::function<void(uint8_t **)> delete_buffer;

    protected:
        std::vector<uint8_t> storage;
    };
} // namespace packunpack

namespace packunpack::server_side
{
    //   'param' classes are modelled after requirements of HBSE* components.
    //   Every 'param' class matches one kind of IPC parameter (documented below).
    //   The same classes are used on both
    // client and server sides. Their implementations differ (they are in different
    // namespaces) and your are responsible to have equal param classes in the same
    // order on both sides.
    //   In server_side::'params', params are deserialized in constructor, while in
    //   'pack_changes' out params serialize
    // themselves.
    template <size_t CALL_PARAMS>
    class param_base
    {
    public:
        constexpr size_t get_out_size() const { return 0; }

        void pack_changes(output_params_stream &ops)
        {
            (void)ops;
        } // no need to be virtual as the base class is never accessed directly

        // how many function call parameters are handled by this class (for example,
        // buffers have two, a pointer and a size, while PODs are just 1)
        static constexpr size_t call_params = CALL_PARAMS;
    };
    // describes function parameters; pod is any simple data type
    // direction 'in' is towards server; 'out' is toward client
    template <typename POD>
    class pod_in : public param_base<1>
    {
        // in: pod, const pod &
        static_assert(is_pod_v<POD>);

    public:
        pod_in(input_params_stream &ips) : ptr(ips.gimme<POD>()) {}
        operator const POD &() { return *ptr; }

    protected:
        const POD *ptr;
    };
    template <typename POD>
    class pod_out : public param_base<1>
    {
        // out: pod &
        static_assert(is_pod_v<POD>);

    public:
        pod_out(input_params_stream &ips) { (void)ips; }
        operator POD &() { return data; }
        constexpr size_t get_out_size() const { return sizeof(data); }
        void pack_changes(output_params_stream &ops) { ops.append(data); }

    protected:
        POD data{};
    };
    template <typename POD = uint8_t, typename SIZE = uint32_t>
    class buffer_in : public param_base<2>
    {
        // in: (const pod *, uint32_t)
        static_assert(is_pod_v<POD>);

    public:
        buffer_in(input_params_stream &ips)
        {
            data_is_nullptr = *ips.gimme<bool>();
            size = *ips.gimme<SIZE>();
            if (!data_is_nullptr && size > 0)
                data = (const POD *)ips.gimme(size * sizeof(POD));
        }
        operator const POD *() { return data; }
        operator SIZE() { return size; } // element count

    protected:
        const POD *data = nullptr;
        bool data_is_nullptr = true;
        SIZE size = 0;
    };
    template <typename POD = uint8_t, typename SIZE = uint32_t>
    class buffer_out : public param_base<2>
    {
        // out: (pod *, uint32_t); size/count of elements is specified by client
        static_assert(is_pod_v<POD>);

    public:
        buffer_out(input_params_stream &ips)
        {
            data_is_nullptr = *ips.gimme<bool>();
            size = *ips.gimme<SIZE>();
            if (!data_is_nullptr && (size > 0))
            {
                data = (POD *)malloc(size * sizeof(POD));
                new (data) POD[size]{};
            }
        }
        buffer_out(buffer_out &&other) : data(std::exchange(other.data, nullptr)), size(other.size) {}
        ~buffer_out() { clear(); }
        operator POD *() { return data; }
        operator SIZE() { return size; }
        constexpr size_t get_out_size() const { return sizeof(*data) * size; }
        void pack_changes(output_params_stream &ops)
        {
            if (data && (size > 0))
                ops.append(data, data + size);
            clear();
        }
        void clear()
        {
            if (!data)
                return;
            for (SIZE i = 0; i < size; ++i)
                data[i].~POD();
            free(data);
            data = nullptr;
            size = 0;
        }

    protected:
        POD *data = nullptr;
        bool data_is_nullptr = true;
        SIZE size = 0; // count of elements
    };
    template <typename POD = uint8_t, typename SIZE = uint32_t, bool DEALLOCATE = true>
    class allocated_buffer_out : public param_base<2>
    {
        // out: (pod **, uint32_t &); size is given by server, buffer needs to be
        // deallocated
        static_assert(is_pod_v<POD>);

    public:
        allocated_buffer_out(input_params_stream &ips) { data_is_nullptr = *ips.gimme<bool>(); }
        allocated_buffer_out(allocated_buffer_out &&other) :
            data(std::exchange(other.data, nullptr)), data_is_nullptr(other.data_is_nullptr), size(other.size)
        {
        }
        operator POD **() { return data_is_nullptr ? nullptr : &data; }
        operator SIZE &() { return size; }
        constexpr size_t get_out_size() const { return sizeof(size) + sizeof(*data) * size; }
        void pack_changes(output_params_stream &ops)
        {
            // assert(data_is_nullptr || !data || (size > 0));
            ops.append(size);
            if (!data_is_nullptr && (size > 0))
                ops.append(data, data + size);

            if constexpr (DEALLOCATE)
            {
                if (data)
                {
                    assert(ops.delete_buffer);
                    ops.delete_buffer(&data);
                }
            }
        }

    protected:
        POD *data = nullptr;
        bool data_is_nullptr = true;
        SIZE size = 0; // element count
    };

    namespace
    {
        // make_tuple_recursive makes a single tuple that holds all 'param' instances we
        // have. They are all constructed with the same param 'C &c' (which will be an
        // input_params_stream).
        template <typename C>
        inline auto make_tuple_recursive(C &)
        {
            return std::make_tuple();
        }
        template <typename C, typename PARAM, typename... PARAMS>
        inline auto make_tuple_recursive(C &c)
        {
            auto t = std::make_tuple<PARAM>(c); // enforce correct order
            return std::tuple_cat(std::move(t), make_tuple_recursive<C, PARAMS...>(c));
        }

        //   Target function parameters are emitted using implicit casts handled by
        //   operator functions from
        // 'param' instances.
        //   make_params will prepare parameters for the target function (as a tuple).
        //   Parameter will be a reference to a 'param' class on which operator function
        //   will be implicitly
        // called to produce a correct parameter.
        // If 'param' class represents more than one parameter in the target function
        // (such as buffer pointer and size), make_params will duplicate those
        // references according to the call_params constant.
        template <size_t I = 0, size_t _COPIES = size_t(-1), typename TUPLE>
        inline auto make_params(TUPLE &t)
        {
            if constexpr (I < std::tuple_size_v<TUPLE>)
            {
                constexpr auto COPIES = (_COPIES != size_t(-1)) ? _COPIES : std::tuple_element_t<I, TUPLE>::call_params;
                if constexpr (COPIES == 0)
                    return make_params<I + 1>(t); // skip this param
                else if constexpr (COPIES == 1)
                    return std::tuple_cat(std::tie(std::get<I>(t)), make_params<I + 1>(t));
                else
                    return std::tuple_cat(std::tie(std::get<I>(t)), make_params<I, COPIES - 1>(t)); // another copy
            }
            else
                return std::make_tuple();
        }

        // An example:
        //   status function(int param1, uint8_t *buffer, uint32_t buffer_size)
        // is modeled as pod_in<int>, buffer_out<uint8_t, uint32_t>.
        // make_tuple_recursive will generate:
        //   std::tuple<pod_in<int>, buffer_out<uint8_t, uint32_t>>{param_stream,
        //   param_stream}
        // make_params will generate:
        //   p = std::tuple<pod_in<int>&, buffer_out<uint8_t, uint32_t>&,
        //   buffer_out<uint8_t, uint32_t>&>
        // handler will call destination as:
        //   std::apply(function_target, p)
        // which will be implicitly executed as:
        //   function(std::get<0>(p).operator const int &, std::get<1>(p).operator
        //   uint8_t *(), std::get<1>(p).operator uint32_t())
    } // namespace

    // dispatcher is just a holder of a few static functions we need templatized in
    // the same way.
    template <typename CMDS /*= CryptoCommands*/, typename INTERFACE /*= ISecureElement **/,
              typename RETURN_STATUS /*= CryptExecStatus*/,
              RETURN_STATUS RETURN_INVALID_COMMAND /*= STATUS::Invalid_Command*/,
              RETURN_STATUS RETURN_EXECUTE_ERROR /*= Status::Execute_Error*/,
              RETURN_STATUS RETURN_EXECUTE_EXCEPTION /*= Status::Execute_Exception*/>
    struct dispatcher
    {
        using reply_t = std::tuple<RETURN_STATUS, std::vector<uint8_t>>;

        static auto &get_handlers()
        {
            static std::unordered_map<CMDS, std::function<reply_t(INTERFACE, input_params_stream &)>> handlers;
            return handlers;
        }

        // links a member INTERFACE function to some command
        template <typename... PARAMS, typename TARGET>
        static void add_handler(CMDS cmd, TARGET target)
        {
            auto &handlers = get_handlers();
            assert(handlers.find(cmd) == handlers.end());
            handlers.emplace(cmd, [target](INTERFACE se, input_params_stream &ps) -> reply_t {
                try
                {
                    auto params_description = make_tuple_recursive<input_params_stream, PARAMS...>(ps);
                    auto params = make_params<>(params_description);
                    if (!ps.empty())
                        throw std::exception(); // something's wrong
                    auto ret = std::apply(target,
                                          std::tuple_cat(std::make_tuple(se),
                                                         std::move(params))); // call target with prepared 'params'

                    // prepare reply
                    output_params_stream buffer([se](uint8_t **buffer) {
                        se->Delete_Buffer(buffer); // too dirty; think of something nicer some day
                    });
                    buffer.reserve(params_description);
                    std::apply([&buffer](auto &...p) { (p.pack_changes(buffer), ...); },
                               params_description); // calls pack_changes for every param we
                                                    // have
                    return {ret, buffer.steal_storage()};
                }
                catch (const std::exception & /*e*/)
                {
                    return {RETURN_EXECUTE_EXCEPTION, {}};
                }
            });
        }

        // finds matching command handler, unpacks parameters and packs outputs
        static reply_t dispatch(CMDS cmd, const uint8_t *buffer, uint32_t buffer_length, INTERFACE executor)
        {
            auto &handlers = get_handlers();
            auto it = handlers.find(cmd);
            assert(it != handlers.end());
            if (it == handlers.end())
                return {RETURN_INVALID_COMMAND, {}};

            try
            {
                input_params_stream ps{buffer, buffer + buffer_length};
                return it->second(executor, ps);
            }
            catch (const std::exception & /*e*/)
            {
                return {RETURN_EXECUTE_EXCEPTION, {}};
            }
        }
    };
} // namespace packunpack::server_side

namespace packunpack::client_side
{
    // The same classes as before, but the implementation for the client side.
    // 'pack_param' with serialize parameter for the call to the server.
    // 'unpack_changes' will deserialize outputs from the server.
    class param_base
    {
    public:
        constexpr size_t get_in_size() const { return 0; }
        void pack_param(output_params_stream &ops)
        {
            (void)ops;
        } // no need to be virtual as base class is never accessed directly
        void unpack_changes(input_params_stream &ips) { (void)ips; }
    };
    template <typename POD>
    class pod_in : public param_base
    {
        // in: pod, const pod &
        static_assert(is_pod_v<POD>);

    public:
        explicit pod_in(const POD &var) : var(var) {}
        constexpr size_t get_in_size() const { return sizeof(POD); }
        void pack_param(output_params_stream &ops) { ops.append(var); }

    protected:
        const POD &var;
    };
    template <typename POD>
    class pod_out : public param_base
    {
        // out: pod &
        static_assert(is_pod_v<POD>);

    public:
        explicit pod_out(POD &var) : var(var) {}
        void unpack_changes(input_params_stream &ips) { var = *ips.gimme<POD>(); }

    protected:
        POD &var;
    };
    template <typename POD = uint8_t, typename SIZE = uint32_t>
    class buffer_in : public param_base
    {
        // in: (const pod *, uint)
        static_assert(is_pod_v<POD>);

    public:
        explicit buffer_in(const POD *ptr, SIZE size) : ptr(ptr), size(size) {}
        buffer_in(std::vector<POD> &&buffer) : storage(std::move(buffer))
        {
            ptr = storage.empty() ? nullptr : storage.data();
            size = (SIZE)storage.size();
        }
        constexpr size_t get_in_size() const { return sizeof(size) + sizeof(*ptr) * size; }
        void pack_param(output_params_stream &ops)
        {
            ops.append<bool>(!ptr);
            ops.append(size);
            if (ptr && (size > 0))
                ops.append(ptr, ptr + size);

            if (!storage.empty())
            {
                ptr = nullptr;
                size = 0;
                decltype(storage)().swap(storage);
            }
        }

    protected:
        const POD *ptr;
        SIZE size; // element count
        std::vector<POD> storage;
    };
    template <typename POD = uint8_t, typename SIZE = uint32_t>
    class buffer_out : public param_base
    {
        // out: (pod *, uint)
        static_assert(is_pod_v<POD>);

    public:
        explicit buffer_out(POD *ptr, SIZE size) : ptr(ptr), size(size) {}
        constexpr size_t get_in_size() const { return sizeof(size); }
        void pack_param(output_params_stream &ops)
        {
            ops.append<bool>(!ptr);
            ops.append(size);
        }
        void unpack_changes(input_params_stream &ips)
        {
            if (ptr && (size > 0))
                memcpy(ptr, ips.gimme(size * sizeof(*ptr)), size * sizeof(*ptr));
        }

    protected:
        POD *const ptr;
        const SIZE size; // element count
    };
    template <typename POD = uint8_t, typename SIZE = uint32_t>
    class allocated_buffer_out : public param_base
    {
        // out: (pod **, uint &)
        static_assert(is_pod_v<POD>);

    public:
        explicit allocated_buffer_out(POD **ptr, SIZE &size) : ptr(ptr), size(size) {}
        void pack_param(output_params_stream &ops) { ops.append<bool>(!ptr); }
        void unpack_changes(input_params_stream &ips)
        {
            size = *ips.gimme<SIZE>();
            if (ptr)
            {
                if (size > 0)
                {
                    *ptr = new POD[size];
                    memcpy(*ptr, ips.gimme(size * sizeof(POD)), size * sizeof(POD));
                }
                else
                    *ptr = nullptr;
            }
        }

    protected:
        POD **ptr;
        SIZE &size; // element count
    };

    template <typename CMDS /*= CryptoCommands*/, typename RETURN_STATUS /*= CryptExecStatus*/,
              RETURN_STATUS RETURN_EXECUTE_ERROR /*= STATUS::Execute_Error*/,
              RETURN_STATUS RETURN_EXECUTE_EXCEPTION /*= STATUS::Execute_Exception*/,
              RETURN_STATUS RETURN_NO_MORE_SESSIONS /*= STATUS::No_More_Sessions*/>
    struct dispatcher
    {
        using reply_t = std::tuple<RETURN_STATUS, std::vector<uint8_t>>;

        // A function that does everything - calls command, packs given input
        // parameters, sends raw message with 'send_to_server' and unpacks the
        // response to given parameters.
        template <typename... PARAMS>
        static RETURN_STATUS SendToServer(CMDS cmd,
                                          const std::function<reply_t(CMDS, std::vector<uint8_t> &&)> *send_to_server,
                                          PARAMS &&...params)
        {
            if (!send_to_server)
                return RETURN_EXECUTE_ERROR;

            try
            {
                output_params_stream send_buffer;
                send_buffer.reserve({(params.get_in_size())...});
                (params.pack_param(send_buffer), ...);

#ifdef USE_PACKET_CALLBACK
                if (get_packet_callback())
                    get_packet_callback()(send_buffer.get_storage(), nullptr);
#endif

                assert(*send_to_server);
                auto [return_code, response_buffer] = (*send_to_server)(cmd, send_buffer.steal_storage());

#ifdef USE_PACKET_CALLBACK
                if (get_packet_callback())
                    get_packet_callback()(response_buffer, &return_code);
#endif

                if ((return_code != RETURN_NO_MORE_SESSIONS) && (return_code != RETURN_EXECUTE_EXCEPTION))
                {
                    input_params_stream received_buffer(response_buffer);
                    (params.unpack_changes(received_buffer), ...);

                    if (!received_buffer.empty())
                        throw std::exception(); // something's wrong
                }
                return return_code;
            }
            catch (const std::exception & /*e*/)
            {
                return RETURN_EXECUTE_EXCEPTION;
            }
        }

#ifdef USE_PACKET_CALLBACK
        static std::function<void(std::vector<uint8_t> &, RETURN_STATUS *)> &get_packet_callback()
        {
            static std::function<void(std::vector<uint8_t> &, RETURN_STATUS *)> packet_callback;
            return packet_callback;
        }
#endif
    };
} // namespace packunpack::client_side
