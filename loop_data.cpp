#include "game.hpp"

#include <cstring> // memcpy, memset


LoopData::LoopData(size_t data_size, InitFnPtr *init_fn, DeinitFnPtr *deinit_fn, LoopCallbackFnPtr *loop_callback_fn)
            : m_raw_data(new unsigned char[data_size]), m_init_fn(init_fn), m_deinit_fn(deinit_fn), m_loop_callback_fn(loop_callback_fn)
{
    //TODO maybe print error when pointer is NULL
    // printf("LoopData constructor called! m_raw_data: %p, init_fn: %p, deinit_fn: %p, loop_callback_fn: %p\n",
    //         m_raw_data, m_init_fn, m_deinit_fn, m_loop_callback_fn);
}

LoopData::LoopData(LoopData&& other)
{
    memcpy(this, &other, sizeof(*this));
    memset(&other, 0, sizeof(other));
}

LoopData::~LoopData()
{
    // printf("LoopData destructor called! m_raw_data: %p, init_fn: %p, deinit_fn: %p, loop_callback_fn: %p\n",
    //         m_raw_data, m_init_fn, m_deinit_fn, m_loop_callback_fn);
    delete[] m_raw_data;
}

void* LoopData::getData() const
{
    return static_cast<void*>(m_raw_data);
}

bool LoopData::isInitialized() const
{
    return getData() != NULL;
}

int LoopData::init() const
{
    if (m_init_fn != NULL)
    {
        return m_init_fn(getData());
    }

    return 0;
}

void LoopData::deinit() const
{
    if (m_deinit_fn != NULL)
    {
        m_deinit_fn(getData());
    }
}

LoopRetVal LoopData::loop_callback() const
{
    if (m_loop_callback_fn != NULL)
    {
        return m_loop_callback_fn(getData());
    }
    
    return LoopRetVal::exit; //TODO other value instead?
}
