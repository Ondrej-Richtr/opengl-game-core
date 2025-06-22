#include "game.hpp"

#include <cstring> // memcpy, memset


MainLoopStack MainLoopStack::instance{};

LoopData::LoopData(size_t data_size, InitFnPtr *init_fn, DeinitFnPtr *deinit_fn, LoopCallbackFnPtr *loop_callback_fn)
            : m_raw_data(std::make_unique<unsigned char[]>(data_size)), m_init_fn(init_fn), m_deinit_fn(deinit_fn), m_loop_callback_fn(loop_callback_fn)
{
    //TODO maybe print error when m_raw_data pointer is NULL
    // printf("LoopData constructor called! m_raw_data: %p, init_fn: %p, deinit_fn: %p, loop_callback_fn: %p\n",
    //         m_raw_data.get(), m_init_fn, m_deinit_fn, m_loop_callback_fn);
}

LoopData::LoopData(LoopData&& other)
{
    memcpy(this, &other, sizeof(*this));
    memset(&other, 0, sizeof(other));
}

LoopData::~LoopData()
{
    // printf("LoopData destructor called! m_raw_data: %p, init_fn: %p, deinit_fn: %p, loop_callback_fn: %p\n",
    //         m_raw_data.get(), m_init_fn, m_deinit_fn, m_loop_callback_fn);
    if (dataInitialized()) deinitAndFree();
}

void* LoopData::getData() const
{
    return static_cast<void*>(m_raw_data.get());
}

bool LoopData::dataInitialized() const
{
    return getData() != NULL;
}

int LoopData::init() const
{
    assert(dataInitialized());

    if (m_init_fn != NULL)
    {
        return m_init_fn(getData());
    }

    return 0;
}

void LoopData::deinit() const
{
    assert(dataInitialized());

    if (m_deinit_fn != NULL)
    {
        m_deinit_fn(getData());
    }
}

void LoopData::deinitAndFree()
{
    assert(dataInitialized());

    deinit();

    m_raw_data.release();
    assert(!dataInitialized());
}

LoopRetVal LoopData::loopCallback(double frame_time, float frame_delta) const
{
    assert(dataInitialized());

    if (m_loop_callback_fn != NULL)
    {
        return m_loop_callback_fn(getData(), frame_time, frame_delta);
    }
    
    return LoopRetVal::exit; //TODO other value instead?
}

const LoopData* MainLoopStack::currentLoopData() const
{
    return m_stack.empty() ? NULL : &m_stack.back();
}

LoopData* MainLoopStack::push(LoopData&& new_data)
{
    if (!new_data.dataInitialized()) return NULL;

    try
    {
        m_stack.emplace_back(std::move(new_data));
    }
    catch (...)
    {
        return NULL;
    }

    assert(!m_stack.empty());
    return &m_stack.back();
}

void MainLoopStack::pop()
{
    if (!m_stack.empty())
    {
        m_stack.pop_back();
    }
}

float MainLoopStack::getFrameDelta(double frame_time)
{
    float frame_delta = 0.f;
    if (m_last_frame_time >= 0.f)
    {
        frame_delta = frame_time - m_last_frame_time;
    }

    m_last_frame_time = frame_time;

    return frame_delta;
}
