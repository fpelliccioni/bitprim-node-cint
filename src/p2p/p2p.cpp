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

#include <bitprim/nodecint/p2p/p2p.h>
//#include <cstdio>
//#include <memory>
//#include <boost/thread/latch.hpp>
//#include <inttypes.h>   //TODO: Remove, it is for the printf (printing pointer addresses)
//#include <cinttypes>   //TODO: Remove, it is for the printf (printing pointer addresses)

#include <bitcoin/network/p2p.hpp>




namespace {

inline
libbitcoin::network::p2p& p2p_cast(p2p_t p2p) {
    return *static_cast<libbitcoin::network::p2p*>(p2p);
}

} /* end of anonymous namespace */


extern "C" {

BITPRIM_EXPORT
uint64_t /*size_t*/ p2p_address_count(p2p_t p2p) {
    return p2p_cast(p2p).address_count();
}

} /* extern "C" */