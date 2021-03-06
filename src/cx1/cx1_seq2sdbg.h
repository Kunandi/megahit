/*
 *  MEGAHIT
 *  Copyright (C) 2014 - 2015 The University of Hong Kong & L3 Bioinformatics Limited
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* contact: Dinghua Li <dhli@cs.hku.hk> */

#ifndef CX1_SEQUENCES2SDBG_H__
#define CX1_SEQUENCES2SDBG_H__

#include <sequence/sequence_package.h>
#include <stdint.h>
#include <mutex>
#include <string>
#include <vector>
#include "cx1.h"
#include "sdbg/sdbg_writer.h"

struct seq2sdbg_opt_t {
  double host_mem;
  double gpu_mem;
  int num_cpu_threads;
  int num_output_threads;
  int kmer_k;
  int kmer_from;
  std::string contig;
  std::string bubble_seq;
  std::string addi_contig;
  std::string local_contig;
  std::string input_prefix;
  int num_edge_files;
  std::string output_prefix;
  int mem_flag;
  bool need_mercy;

  seq2sdbg_opt_t() {
    host_mem = 0;
    gpu_mem = 0;
    num_cpu_threads = 0;
    num_output_threads = 0;
    num_edge_files = 0;
    kmer_k = 0;
    kmer_from = 0;
    mem_flag = 1;
    contig = "";
    addi_contig = "";
    local_contig = "";
    input_prefix = "";
    output_prefix = "out";
    need_mercy = false;
  }
};

namespace cx1_seq2sdbg {

static const int kBucketPrefixLength = 8;  // less than 16 (chars per word)
static const int kBucketBase = 4;
static const int kNumBuckets = 65536;  // pow(4, 8)
// binary search look up table
static const int kLookUpPrefixLength = 12;
static const int kLookUpShift = 32 - kLookUpPrefixLength * 2;
static const int kLookUpSize = 1 << (2 * kLookUpPrefixLength);

static const int64_t kDefaultLv1ScanTime = 8;
static const int64_t kMaxLv1ScanTime = 64;
static const int kSentinelValue = 4;
static const int kBWTCharNumBits = 3;

struct seq2sdbg_global_t {
  CX1<seq2sdbg_global_t, kNumBuckets> cx1;

  // input options
  int kmer_k;
  int kmer_from;
  int num_cpu_threads;
  int num_output_threads;
  int64_t host_mem;
  int mem_flag;
  bool need_mercy;

  std::string contig;
  std::string bubble_seq;
  std::string addi_contig;
  std::string local_contig;
  std::string input_prefix;
  std::string output_prefix;

  int64_t num_seq;
  int64_t words_per_substring;  // substrings to be sorted by GPU
  int64_t max_bucket_size;
  int64_t tot_bucket_size;
  int words_per_dummy_node;

  // big arrays
  SeqPackage package;
  std::vector<mul_t> multiplicity;
  std::vector<int32_t> lv1_items;

  int64_t max_sorting_items;

  std::mutex lv1_items_scanning_lock;
  // memory usage
  int64_t mem_packed_seq;

  // output
  SdbgWriter sdbg_writer;
};

int64_t encode_lv1_diff_base(int64_t read_id, seq2sdbg_global_t &g);
void read_seq_and_prepare(
    seq2sdbg_global_t &g);           // num_items_, num_cpu_threads_ and num_output_threads_ must be set here
void *lv0_calc_bucket_size(void *);  // pthread working function
void init_global_and_set_cx1(seq2sdbg_global_t &g);
void *lv1_fill_offset(void *);  // pthread working function
void lv1_direct_sort_and_proc(seq2sdbg_global_t &g);
void post_proc(seq2sdbg_global_t &g);

}  // end of namespace cx1_seq2sdbg
#endif  // CX1_SEQUENCES2SDBG_H__