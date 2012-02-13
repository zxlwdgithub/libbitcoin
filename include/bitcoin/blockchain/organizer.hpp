#ifndef LIBBITCOIN_BLOCKCHAIN_ORGANIZER_H
#define LIBBITCOIN_BLOCKCHAIN_ORGANIZER_H

#include <boost/circular_buffer.hpp>

#include <bitcoin/block.hpp>
#include <bitcoin/messages.hpp>
#include <bitcoin/blockchain/blockchain.hpp>
#include <bitcoin/utility/big_number.hpp>

namespace libbitcoin {

// Metadata + block
class block_detail
{
public:
    block_detail(const message::block& actual_block);
    const message::block& actual() const;
    std::shared_ptr<message::block> actual_ptr() const;
    void mark_processed();
    bool is_processed();
    const hash_digest& hash() const;
    void set_info(const block_info& replace_info);
    const block_info& info() const;
private:
    std::shared_ptr<message::block> actual_block_;
    const hash_digest block_hash_;
    bool processed_;
    block_info info_;
};

typedef std::shared_ptr<block_detail> block_detail_ptr;
typedef std::vector<block_detail_ptr> block_detail_list;

// An unordered memory pool for orphan blocks
class orphans_pool
{
public:
    orphans_pool(size_t pool_size);
    void add(block_detail_ptr incoming_block);
    block_detail_list trace(block_detail_ptr end_block);
    block_detail_list unprocessed();
    void remove(block_detail_ptr remove_block);
private:
    boost::circular_buffer<block_detail_ptr> pool_;
};

typedef std::shared_ptr<orphans_pool> orphans_pool_ptr;

// The actual blockchain is encapsulated by this
class chain_keeper
{
public:
    virtual void start() = 0;
    // Must be able to call stop() twice without problems
    virtual void stop() = 0;

    virtual void add(block_detail_ptr incoming_block) = 0;
    virtual int find_index(const hash_digest& search_block_hash) = 0;
    virtual big_number end_slice_difficulty(size_t slice_begin_index) = 0;
    virtual block_detail_list end_slice(size_t slice_begin_index) = 0;
};

typedef std::shared_ptr<chain_keeper> chain_keeper_ptr;

// Structure which organises the blocks from the orphan pool to the blockchain
class organizer
{
public:
    organizer(orphans_pool_ptr orphans, chain_keeper_ptr chain);

    void start();

protected:
    virtual bool verify(int fork_index,
        const block_detail_list& orphan_chain, int orphan_index) = 0;
    virtual void reorganize_occured(
        const blockchain::block_list& arrivals,
        const blockchain::block_list& replaced) = 0;

private:
    void process(block_detail_ptr process_block);
    void replace_chain(int fork_index, block_detail_list& orphan_chain);
    void clip_orphans(block_detail_list& orphan_chain, int orphan_index);
    void notify_reorganize(
        const block_detail_list& orphan_chain,
        const block_detail_list& replaced_slice);

    orphans_pool_ptr orphans_;
    chain_keeper_ptr chain_;

    block_detail_list process_queue_;
};

typedef std::shared_ptr<organizer> organizer_ptr;

} // libbitcoin

#endif

