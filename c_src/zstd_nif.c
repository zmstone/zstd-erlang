#include "erl_nif.h"

#include <stdlib.h>
#include <zstd.h>

static ERL_NIF_TERM zstd_nif_compress(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  ErlNifBinary bin, ret_bin;
  size_t buff_size, compressed_size;
  unsigned int compression_level;

  if(!enif_inspect_binary(env, argv[0], &bin)
     || !enif_get_uint(env, argv[1], &compression_level)
     || compression_level > ZSTD_maxCLevel())
    return enif_make_badarg(env);

  buff_size = ZSTD_compressBound(bin.size);

  if(!enif_alloc_binary(buff_size, &ret_bin))
    return enif_make_atom(env, "error");

  compressed_size = ZSTD_compress(ret_bin.data, buff_size, bin.data, bin.size, compression_level);
  if(ZSTD_isError(compressed_size))
    return enif_make_atom(env, "error");

  if(!enif_realloc_binary(&ret_bin, compressed_size))
    return enif_make_atom(env, "error");

  return enif_make_binary(env, &ret_bin);
}

static ERL_NIF_TERM zstd_nif_decompress(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  ERL_NIF_TERM out;
  unsigned char *outp;
  ErlNifBinary bin;
  unsigned long long uncompressed_size;

  if(!enif_inspect_binary(env, argv[0], &bin))
    return enif_make_badarg(env);

  uncompressed_size = ZSTD_getDecompressedSize(bin.data, bin.size);

  outp = enif_make_new_binary(env, uncompressed_size, &out);

  if(ZSTD_decompress(outp, uncompressed_size, bin.data, bin.size) != uncompressed_size)
    return enif_make_badarg(env);

  return out;
}

static ErlNifFunc nif_funcs[] = {
    {"compress_bin", 2, zstd_nif_compress},
    {"decompress", 1, zstd_nif_decompress}
};

ERL_NIF_INIT(zstd, nif_funcs, NULL, NULL, NULL, NULL);
