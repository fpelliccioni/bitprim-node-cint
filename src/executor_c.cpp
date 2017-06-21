/**
 * Copyright (c) 2017 Bitprim developers (see AUTHORS)
 *
 * This file is part of Bitprim.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <bitprim/nodecint/executor_c.h>
#include <cstdio>
#include <memory>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/thread/latch.hpp>
#include <bitprim/nodecint/executor.hpp>

libbitcoin::node::configuration make_config(char const* path) {
    libbitcoin::node::configuration config(libbitcoin::config::settings::mainnet);
    config.file = boost::filesystem::path(path);
    return config;
}




inline
FILE* devnull_file() {
#ifdef BOOST_IOSTREAMS_WINDOWS
    return fopen ("nul", "w");
#else
    return fopen ("/dev/null", "w");
#endif
}

inline
int devnull_fileno() {
    return fileno(devnull_file());
}

extern "C" {

struct executor {

    executor(char const* path)
        :
        sout_buffer_(boost::iostreams::file_descriptor_sink(devnull_fileno(), boost::iostreams::never_close_handle))
        , serr_buffer_(boost::iostreams::file_descriptor_sink(devnull_fileno(), boost::iostreams::never_close_handle))
        , sout_(&sout_buffer_)
        , serr_(&serr_buffer_)
        , actual(make_config(path), sout_, serr_)
    {
//        std::ostream os(&sout_buffer_);
//        os << "Hello World!" << std::endl;
    }

    executor(char const* path, FILE* sout, FILE* serr)
        :
        sout_buffer_(boost::iostreams::file_descriptor_sink(fileno(sout), boost::iostreams::never_close_handle))
        , serr_buffer_(boost::iostreams::file_descriptor_sink(fileno(serr), boost::iostreams::never_close_handle))
        , sout_(&sout_buffer_)
        , serr_(&serr_buffer_)
        , actual(make_config(path), sout_, serr_)
    {
//        std::ostream os(&sout_buffer_);
//        os << "Hello World!" << std::endl;
    }

    executor(char const* path, int sout_fd, int serr_fd)
        :
//        sin_buffer_(boost::iostreams::file_descriptor_source(sin_fd, boost::iostreams::never_close_handle))
          sout_buffer_(boost::iostreams::file_descriptor_sink(sout_fd, boost::iostreams::never_close_handle))
          , serr_buffer_(boost::iostreams::file_descriptor_sink(serr_fd, boost::iostreams::never_close_handle))
//          , sin_(&sin_buffer_)
          , sout_(&sout_buffer_)
          , serr_(&serr_buffer_)
          , actual(make_config(path), sout_, serr_)
    {
        std::ostream os(&sout_buffer_);
        os << "Hello World -- 2!" << std::endl;
    }

#ifdef BOOST_IOSTREAMS_WINDOWS
    using handle_source = typename boost::iostreams::file_descriptor_source::handle_type;
    using handle_sink = typename boost::iostreams::file_descriptor_sink::handle_type;

    executor(char const* path, handle_sink sout, handle_sink serr)
        :
//        sin_buffer_(boost::iostreams::file_descriptor_source(sin, boost::iostreams::never_close_handle))
        sout_buffer_(boost::iostreams::file_descriptor_sink(sout, boost::iostreams::never_close_handle))
        , serr_buffer_(boost::iostreams::file_descriptor_sink(serr, boost::iostreams::never_close_handle))
//        , sin_(&sin_buffer_)
        , sout_(&sout_buffer_)
        , serr_(&serr_buffer_)
        , actual(make_config(path), sout_, serr_)
    {
        std::ostream os(&sout_buffer_);
        os << "Hello World -- 3!" << std::endl;
    }
#endif /* BOOST_IOSTREAMS_WINDOWS */

//    boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_source> sin_buffer_;
    boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_sink> sout_buffer_;
    boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_sink> serr_buffer_;

//    std::istream sin_;
    std::ostream sout_;
    std::ostream serr_;

    bitprim::nodecint::executor actual;
};

executor_t executor_construct_devnull(char const* path) {
    return std::make_unique<executor>(path).release();
}

executor_t executor_construct(char const* path, FILE* sout, FILE* serr) {
    return std::make_unique<executor>(path, sout, serr).release();
}

executor_t executor_construct_fd(char const* path, int sout_fd, int serr_fd) {
    return std::make_unique<executor>(path, sout_fd, serr_fd).release();
}

#ifdef BOOST_IOSTREAMS_WINDOWS

executor_t executor_construct_handles(char const* path, void* sout, void* serr) {
    return std::make_unique<executor>(path, sout, serr).release();
}

#endif /* BOOST_IOSTREAMS_WINDOWS */


void executor_destruct(executor_t exec) {
//    std::cout << "From C++: executor_destruct\n";
//    printf("executor_destruct - ex
// ec: 0x%" PRIXPTR "\n", (uintptr_t)exec);

    delete exec;
}

int executor_initchain(executor_t exec) {
    return exec->actual.do_initchain();
}

int executor_run(executor_t exec) {
    return exec->actual.run();
}

int executor_run_wait(executor_t exec) {
    return exec->actual.run_wait();
}

void executor_stop(executor_t exec) {
    exec->actual.stop();
}

void fetch_last_height(executor_t exec, last_height_fetch_handler_t handler) {
    exec->actual.node().chain().fetch_last_height([handler](std::error_code const& ec, size_t h) {
        handler(ec.value(), h);
    });
}

int get_last_height(executor_t exec, size_t* height) {
    boost::latch latch(2); //Note: workaround to fix an error on some versions of Boost.Threads

    int res;
    exec->actual.node().chain().fetch_last_height([&](std::error_code const& ec, size_t h) {
        *height = h;
        res = ec.value();
        latch.count_down();
    });

    latch.count_down_and_wait();
    return res;
}

void fetch_block_height(executor_t exec, hash_t hash, block_height_fetch_handler_t handler) {

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    exec->actual.node().chain().fetch_block_height(hash_cpp, [handler](std::error_code const& ec, size_t h) {
        handler(ec.value(), h);
    });
}

int get_block_height(executor_t exec, hash_t hash, size_t* height) {
    boost::latch latch(2); //Note: workaround to fix an error on some versions of Boost.Threads
    int res;

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    exec->actual.node().chain().fetch_block_height(hash_cpp, [&](std::error_code const& ec, size_t h) {
        *height = h;
        res = ec.value();
        latch.count_down();
    });

    latch.count_down_and_wait();
    return res;
}

void fetch_block_header(executor_t exec, size_t height, block_header_fetch_handler_t handler) {
    exec->actual.node().chain().fetch_block_header(height, [handler](std::error_code const& ec, libbitcoin::message::header::ptr header, size_t h) {
        auto new_header = new libbitcoin::message::header(*header.get());
//        auto new_header = std::make_unique(*header.get()).release();
        //Note: It is the responsability of the user to release/destruct the object
        handler(ec.value(), new_header, h);
    });
}

int get_block_header(executor_t exec, size_t height, header_t* out_header, size_t* out_height) {
    boost::latch latch(2); //Note: workaround to fix an error on some versions of Boost.Threads
    int res;

    exec->actual.node().chain().fetch_block_header(height, [&](std::error_code const& ec, libbitcoin::message::header::ptr header, size_t h) {
        *out_header = new libbitcoin::message::header(*header.get());
        //Note: It is the responsability of the user to release/destruct the object

        *out_height = h;
        res = ec.value();
        latch.count_down();
    });

    latch.count_down_and_wait();
    return res;
}

void fetch_block_header_by_hash(executor_t exec, hash_t hash, block_header_fetch_handler_t handler) {

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    exec->actual.node().chain().fetch_block_header(hash_cpp, [handler](std::error_code const& ec, libbitcoin::message::header::ptr header, size_t h) {
        auto new_header = new libbitcoin::message::header(*header.get());
//        auto new_header = std::make_unique(*header.get()).release();
        //Note: It is the responsability of the user to release/destruct the object
        handler(ec.value(), new_header, h);
    });
}

int get_block_header_by_hash(executor_t exec, hash_t hash, header_t* out_header, size_t* out_height) {
    boost::latch latch(2); //Note: workaround to fix an error on some versions of Boost.Threads
    int res;

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    exec->actual.node().chain().fetch_block_header(hash_cpp, [&](std::error_code const& ec, libbitcoin::message::header::ptr header, size_t h) {
        *out_header = new libbitcoin::message::header(*header.get());
        //Note: It is the responsability of the user to release/destruct the object

        *out_height = h;
        res = ec.value();
        latch.count_down();
    });

    latch.count_down_and_wait();
    return res;
}

void fetch_block(executor_t exec, size_t height, block_fetch_handler_t handler) {
    exec->actual.node().chain().fetch_block(height, [handler](std::error_code const& ec, libbitcoin::message::block::ptr block, size_t h) {
        auto new_block = new libbitcoin::message::block(*block.get());
//        auto new_block = std::make_unique(*block.get()).release();
        //Note: It is the responsability of the user to release/destruct the object
        handler(ec.value(), new_block, h);
    });
}

int get_block(executor_t exec, size_t height, block_t* out_block, size_t* out_height) {
    boost::latch latch(2); //Note: workaround to fix an error on some versions of Boost.Threads
    int res;

    exec->actual.node().chain().fetch_block(height, [&](std::error_code const& ec, libbitcoin::message::block::ptr block, size_t h) {
        *out_block = new libbitcoin::message::block(*block.get());
        //Note: It is the responsability of the user to release/destruct the object

        *out_height = h;
        res = ec.value();
        latch.count_down();
    });

    latch.count_down_and_wait();
    return res;
}

void fetch_block_by_hash(executor_t exec, hash_t hash, block_fetch_handler_t handler) {

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    exec->actual.node().chain().fetch_block(hash_cpp, [handler](std::error_code const& ec, libbitcoin::message::block::ptr block, size_t h) {
        auto new_block = new libbitcoin::message::block(*block.get());
        //Note: It is the responsability of the user to release/destruct the object
        handler(ec.value(), new_block, h);
    });
}

int get_block_by_hash(executor_t exec, hash_t hash, block_t* out_block, size_t* out_height) {
    boost::latch latch(2); //Note: workaround to fix an error on some versions of Boost.Threads
    int res;

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    exec->actual.node().chain().fetch_block(hash_cpp, [&](std::error_code const& ec, libbitcoin::message::block::ptr block, size_t h) {
        *out_block = new libbitcoin::message::block(*block.get());
        //Note: It is the responsability of the user to release/destruct the object

        *out_height = h;
        res = ec.value();
        latch.count_down();
    });

    latch.count_down_and_wait();
    return res;
}

void fetch_transaction(executor_t exec, hash_t hash, int require_confirmed, transaction_fetch_handler_t handler) {

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    exec->actual.node().chain().fetch_transaction(hash_cpp, require_confirmed, [handler](std::error_code const& ec, libbitcoin::message::transaction::ptr transaction, size_t h, size_t i) {
        auto new_transaction = new libbitcoin::message::transaction(*transaction.get());
        handler(ec.value(), new_transaction, h, i);
    });
}

int get_transaction(executor_t exec, hash_t hash, int require_confirmed, transaction_t* out_transaction, size_t* out_height, size_t* out_index) {
    boost::latch latch(2); //Note: workaround to fix an error on some versions of Boost.Threads
    int res;

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    exec->actual.node().chain().fetch_transaction(hash_cpp, require_confirmed, [&](std::error_code const& ec, libbitcoin::message::transaction::ptr transaction, size_t h, size_t i) {
        *out_transaction = new libbitcoin::message::transaction(*transaction.get());
        *out_height = h;
        *out_index = i;
        res = ec.value();
        latch.count_down();
    });

    latch.count_down_and_wait();
    return res;

}

void fetch_output(executor_t exec, hash_t hash, uint32_t index, int require_confirmed, output_fetch_handler_t handler) {

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    libbitcoin::chain::output_point point(hash_cpp, index);

    exec->actual.node().chain().fetch_output(point, require_confirmed, [handler](std::error_code const& ec, libbitcoin::chain::output const& output) {
        auto new_output = new libbitcoin::chain::output(output);
        handler(ec.value(), new_output);
    });
}

int get_output(executor_t exec, hash_t hash, uint32_t index, int require_confirmed, output_t* out_output) {
    boost::latch latch(2); //Note: workaround to fix an error on some versions of Boost.Threads
    int res;

    libbitcoin::hash_digest hash_cpp;
    std::copy_n(hash, hash_cpp.size(), std::begin(hash_cpp));

    libbitcoin::chain::output_point point(hash_cpp, index);


    exec->actual.node().chain().fetch_output(point, require_confirmed, [&](std::error_code const& ec, libbitcoin::chain::output const& output) {
        *out_output = new libbitcoin::chain::output(output);

        res = ec.value();
        latch.count_down();
    });

    latch.count_down_and_wait();
    return res;
}

} /* extern "C" */
