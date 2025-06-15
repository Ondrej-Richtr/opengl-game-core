#include "game.hpp"

#include <cstring> // memcpy, memset


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

LoopRetVal LoopData::loopCallback() const
{
    assert(dataInitialized());

    if (m_loop_callback_fn != NULL)
    {
        return m_loop_callback_fn(getData());
    }
    
    return LoopRetVal::exit; //TODO other value instead?
}

const LoopData* MainLoopStack::currentLoopData() const
{
    return m_stack.empty() ? NULL : &m_stack.back();
}

bool MainLoopStack::push(LoopData&& new_data)
{
    if (!new_data.dataInitialized()) return false;

    try
    {
        m_stack.emplace_back(std::move(new_data));
    }
    catch (...)
    {
        return false;
    }

    return true;
}

void MainLoopStack::pop()
{
    if (!m_stack.empty())
    {
        m_stack.pop_back();
    }
}
