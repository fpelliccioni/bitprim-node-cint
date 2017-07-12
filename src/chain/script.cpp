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

#include <bitprim/nodecint/script.h>
#include <bitcoin/bitcoin/chain/script.hpp>

extern "C" {

void script_destruct(script_t script) {
    auto script_cpp = static_cast<libbitcoin::chain::script*>(script);
    delete script_cpp;
}

int script_is_valid(script_t script) {
    return static_cast<libbitcoin::chain::script const*>(script)->is_valid();
}

int script_is_valid_operations(script_t script) {
    return static_cast<libbitcoin::chain::script const*>(script)->is_valid_operations();
}

size_t script_satoshi_content_size(script_t script) {
    return static_cast<libbitcoin::chain::script const*>(script)->satoshi_content_size();
}

size_t script_serialized_size(script_t script, /*bool*/ int prefix) {
    return static_cast<libbitcoin::chain::script const*>(script)->serialized_size(prefix);
}

//Note: user of the function has to release the resource (memory) manually
char const* script_to_string(script_t script, uint32_t active_forks) {
    auto str = static_cast<libbitcoin::chain::script const*>(script)->to_string(active_forks);
    char* ret = (char*)malloc((str.size() + 1) * sizeof(char));
    std::strcpy(ret, str.c_str());
    return ret;
}

//std::string (uint32_t active_forks) const;


size_t script_sigops(script_t script, /*bool*/ int embedded) {
    return static_cast<libbitcoin::chain::script const*>(script)->sigops(embedded);
}

size_t script_embedded_sigops(script_t script, script_t prevout_script) {
    auto& prevout_script_cpp = *static_cast<libbitcoin::chain::script const*>(prevout_script);
    return static_cast<libbitcoin::chain::script const*>(script)->embedded_sigops(prevout_script_cpp);
}

//void script_find_and_delete(script_t script, const data_stack& endorsements) {
//    return static_cast<libbitcoin::chain::script const*>(script)->embedded_sigops(find_and_delete);
//}







//
//// Serialization.
////-------------------------------------------------------------------------
//
//data_chunk to_data(bool prefix) const;
//void to_data(std::ostream& stream, bool prefix) const;
//void to_data(writer& sink, bool prefix) const;
//
//std::string to_string(uint32_t active_forks) const;
//
//// Iteration.
////-------------------------------------------------------------------------
//
//void clear();
//bool empty() const;
//size_t size() const;
//const operation& front() const;
//const operation& back() const;
//operation::iterator begin() const;
//operation::iterator end() const;
//const operation& operator[](size_t index) const;
//
//// Properties (size, accessors, cache).
////-------------------------------------------------------------------------
//
//const operation::list& operations() const;
//
//// Signing.
////-------------------------------------------------------------------------
//
//static hash_digest generate_signature_hash(const transaction& tx,
//                                           uint32_t input_index, const script& script_code, uint8_t sighash_type);
//
//static bool check_signature(const ec_signature& signature,
//                            uint8_t sighash_type, const data_chunk& public_key,
//                            const script& script_code, const transaction& tx,
//                            uint32_t input_index);
//
//static bool create_endorsement(endorsement& out, const ec_secret& secret,
//                               const script& prevout_script, const transaction& tx,
//                               uint32_t input_index, uint8_t sighash_type);
//
//// Utilities (static).
////-------------------------------------------------------------------------
//
///// Determine if the fork is enabled in the active forks set.
//static bool is_enabled(uint32_t active_forks, machine::rule_fork fork)
//{
//    return (fork & active_forks) != 0;
//}
//
///// No-code patterns (consensus).
//static bool is_push_only(const operation::list& ops);
//static bool is_relaxed_push(const operation::list& ops);
//static bool is_coinbase_pattern(const operation::list& ops, size_t height);
//
///// Unspendable pattern (standard).
//static bool is_null_data_pattern(const operation::list& ops);
//
///// Payment script patterns (standard, psh is also consensus).
//static bool is_pay_multisig_pattern(const operation::list& ops);
//static bool is_pay_public_key_pattern(const operation::list& ops);
//static bool is_pay_key_hash_pattern(const operation::list& ops);
//static bool is_pay_script_hash_pattern(const operation::list& ops);
//
///// Signature script patterns (standard).
//static bool is_sign_multisig_pattern(const operation::list& ops);
//static bool is_sign_public_key_pattern(const operation::list& ops);
//static bool is_sign_key_hash_pattern(const operation::list& ops);
//static bool is_sign_script_hash_pattern(const operation::list& ops);
//
///// Stack factories (standard).
//static operation::list to_null_data_pattern(data_slice data);
//static operation::list to_pay_public_key_pattern(data_slice point);
//static operation::list to_pay_key_hash_pattern(const short_hash& hash);
//static operation::list to_pay_script_hash_pattern(const short_hash& hash);
//static operation::list to_pay_multisig_pattern(uint8_t signatures,
//                                               const point_list& points);
//static operation::list to_pay_multisig_pattern(uint8_t signatures,
//                                               const data_stack& points);
//
//// Utilities (non-static).
////-------------------------------------------------------------------------
//
//machine::script_pattern pattern() const;
//void find_and_delete(const data_stack& endorsements);
//
//// Validation.
////-------------------------------------------------------------------------
//
//static code verify(const transaction& tx, uint32_t input, uint32_t forks);
//
//// TOD: move back to private.
//static code verify(const transaction& tx, uint32_t input_index,
//                   uint32_t forks, const script& input_script,
//                   const script& prevout_script);

} /* extern "C" */