#pragma once

// -- sio headers
#include <sio/definitions.h>

// -- zlib headers
#include <zlib.h>

namespace sio {
  
  class zlib_compression {
  public:
    /// Default constructor
    zlib_compression() = default ;
    /// Default destructor
    ~zlib_compression() = default ;
    
    /**
     *  @brief  Set the compression level.
     *          - Z_DEFAULT_COMPRESSION: default zlib compression level
     *          - -1: no commpression
     *          - [1-9] various levels
     *          Note that above 9, the level is to 9
     *          
     *  @param  level the compression level to use
     */
    void set_level( int level ) ;
    
    /**
     *  @brief  Get the compression level
     */
    int level() const ;

    /**
     *  @brief  Uncompress the buffer and return a new buffer (reference).
     *          The uncpmpressed buffer must have been resized correctly 
     *          before calling this function.
     * 
     *  @param  inbuf the input buffer to uncompress
     *  @param  outbuf the uncompressed buffer to receive
     */
    void uncompress( const buffer_span &inbuf, buffer &outbuf ) ;
    
    /**
     *  @brief  Compress the buffer and return a new buffer
     * 
     *  @param  inbuf the input buffer to compress
     *  @param  outbuf the output buffer to receive
     */
    void compress( const buffer_span &inbuf, buffer &outbuf ) ;
    
  private:
    ///< The compression level (on compress)
    int               _level {Z_DEFAULT_COMPRESSION} ;
  };
  
  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  
  inline void zlib_compression::set_level( int level ) {
    if(level < 0) {
      _level = Z_DEFAULT_COMPRESSION;
    }
    else if(level > 9) {
      _level = 9;
    }
    else {
      _level = level;
    }
  }
  
  //--------------------------------------------------------------------------
  
  inline int zlib_compression::level() const {
    return _level ;
  }
  
  //--------------------------------------------------------------------------
  
  void zlib_compression::uncompress( const buffer_span &inbuf, buffer &outbuf ) {
    if( not inbuf.valid() ) {
      SIO_THROW( sio::error_code::invalid_argument, "Buffer is not valid" ) ;
    }
    // zlib uncompress requires to pass an address of uLongf type ...
    uLongf outsize = outbuf.size() ;
    auto zstat = ::uncompress( (Bytef*)outbuf.data(), &outsize, (const Bytef*)inbuf.data(), inbuf.size() ) ;
    if( Z_OK != zstat ) {
      std::stringstream ss ;
      ss << "Zlib uncompression failed with status " << zstat ;
      SIO_THROW( sio::error_code::compress_error, ss.str() ) ;
    }
    SIO_DEBUG( "ZLIB uncompress OK!" ) ;
  }
  
  //--------------------------------------------------------------------------
  
  void zlib_compression::compress( const buffer_span &inbuf, buffer &outbuf ) {
    if( not inbuf.valid() ) {
      SIO_THROW( sio::error_code::invalid_argument, "Buffer is not valid" ) ;
    }
    // comp_bound is a first estimate of the compressed size.
    // After compression, the real output size is returned,
    // this is why the buffer is resized after calling compress2().
    auto comp_bound = ::compressBound( inbuf.size() ) ;
    if( outbuf.size() < comp_bound ) {
      outbuf.resize( comp_bound ) ;
    }
    auto zstat = ::compress2( (Bytef*)outbuf.data(), &comp_bound, (const Bytef*)inbuf.data(), inbuf.size(), _level ) ;
    if( Z_OK != zstat ) {
      std::stringstream ss ;
      ss << "Zlib compression failed with status " << zstat ;
      SIO_THROW( sio::error_code::compress_error, ss.str() ) ;
    }
    outbuf.resize( comp_bound ) ;
    SIO_DEBUG( "ZLIB compress OK!" ) ;
  }
}

