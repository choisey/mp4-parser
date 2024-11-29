#ifndef __MP4BOXTYPES_H__
#define __MP4BOXTYPES_H__

#include <stdint.h>

#include <memory>
#include <map>
#include <vector>
#include <string>

// based on ISO/IEC 14496-12:2005(E)

static const uint32_t MP4FILE = 0xffffffff;
static const uint32_t BXML = 0x62786d6c;
static const uint32_t CO64 = 0x636f3634;
static const uint32_t CPRT = 0x63707274;
static const uint32_t CTTS = 0x63747473;
static const uint32_t DINF = 0x64696e66;
static const uint32_t DREF = 0x64726566;
static const uint32_t EDTS = 0x65647473;
static const uint32_t ELST = 0x656c7374;
static const uint32_t FREE = 0x66726565;
static const uint32_t FRMA = 0x66726d61;
static const uint32_t FTYP = 0x66747970;
static const uint32_t HDLR = 0x68646c72;
static const uint32_t HMHD = 0x686d6864;
static const uint32_t IINF = 0x69696e66;
static const uint32_t ILOC = 0x696c6f63;
static const uint32_t IMIF = 0x696d6966;
static const uint32_t INFE = 0x696e6665;
static const uint32_t IPMC = 0x69706d63;
static const uint32_t IPRO = 0x6970726f;
static const uint32_t MDAT = 0x6d646174;
static const uint32_t MDHD = 0x6d646864;
static const uint32_t MDIA = 0x6d646961;
static const uint32_t MEHD = 0x6d656864;
static const uint32_t META = 0x6d657461;
static const uint32_t MFHD = 0x6d666864;
static const uint32_t MFRA = 0x6d667261;
static const uint32_t MFRO = 0x6d66726f;
static const uint32_t MINF = 0x6d696e66;
static const uint32_t MOOF = 0x6d6f6f66;
static const uint32_t MOOV = 0x6d6f6f76;
static const uint32_t MVEX = 0x6d766578;
static const uint32_t MVHD = 0x6d766864;
static const uint32_t NMHD = 0x6e6d6864;
static const uint32_t PADB = 0x70616462;
static const uint32_t PDIN = 0x7064696e;
static const uint32_t PITM = 0x7069746d;
static const uint32_t SBGP = 0x73626770;
static const uint32_t SCHI = 0x73636869;
static const uint32_t SCHM = 0x7363686d;
static const uint32_t SDTP = 0x73647470;
static const uint32_t SGPD = 0x73677064;
static const uint32_t SINF = 0x73696e66;
static const uint32_t SKIP = 0x736b6970;
static const uint32_t SMHD = 0x736d6864;
static const uint32_t SRPP = 0x73727070;
static const uint32_t STBL = 0x7374626c;
static const uint32_t STCO = 0x7374636f;
static const uint32_t STDP = 0x73746470;
static const uint32_t STSC = 0x73747363;
static const uint32_t STSD = 0x73747364;
static const uint32_t STSH = 0x73747368;
static const uint32_t STSL = 0x7374736c;
static const uint32_t STSS = 0x73747373;
static const uint32_t STSZ = 0x7374737a;
static const uint32_t STTS = 0x73747473;
static const uint32_t STZ2 = 0x73747a32;
static const uint32_t SUBS = 0x73756273;
static const uint32_t TFHD = 0x74666864;
static const uint32_t TFRA = 0x74667261;
static const uint32_t TKHD = 0x746b6864;
static const uint32_t TRAF = 0x74726166;
static const uint32_t TRAK = 0x7472616b;
static const uint32_t TREF = 0x74726566;
static const uint32_t TREX = 0x74726578;
static const uint32_t TRUN = 0x7472756e;
static const uint32_t UDTA = 0x75647461;
static const uint32_t VMHD = 0x766d6864;
static const uint32_t XML  = 0x786d6c20;
static const uint32_t URL  = 0x75726c20;
static const uint32_t URN  = 0x75726e20;
static const uint32_t UUID = 0x75756964;

// specific to F4V/HDS

static const uint32_t ABST = 0x61627374;
static const uint32_t ASRT = 0x61737274;
static const uint32_t AFRT = 0x61667274;

// specific to DASH segment

static const uint32_t STYP = 0x73747970;
static const uint32_t SIDX = 0x73696478;
static const uint32_t TFDT = 0x74666474;

static const uint32_t _XX_
	= (uint32_t) '-' * 0x1000000
	+ (uint32_t) '-' * 0x10000
	+ (uint32_t) '-' * 0x100
	+ (uint32_t) '-';

struct BoxHead {
	size_t offset;
	size_t boxheadsize;
	size_t boxsize;
	uint32_t boxtype;

	// full box
	uint8_t version;
	uint32_t flag;

	bool is_fullbox() const;
	size_t size() const {
		return is_fullbox()
			? ( sizeof(uint32_t) * 3 )
			: ( sizeof(uint32_t) * 2 );
	}
};

// ftyp
struct FileTypeBox {
	std::string major_brand;
	std::string minor_version;
	std::vector<std::string> compatible_brands;

	size_t size() const {
		return sizeof(uint32_t) * ( compatible_brands.size() + 2 );
	}
};

// mvhd
struct MovieHeaderBox {
	uint64_t creation_time;
	uint64_t modification_time;
	uint32_t timescale;
	uint64_t duration;
	uint32_t rate;
	int32_t volume;
	std::vector<uint32_t> matrix;
	uint32_t next_track_ID;

	size_t size(uint8_t version) const {
		switch ( version ) {
			case 1:
				return sizeof(uint64_t) * 3
					+ sizeof(uint32_t) * 20
					+ sizeof(uint16_t) * 2;

			default: // version == 0
				return sizeof(uint32_t) * 23
					+ sizeof(uint16_t) * 2;
		}
	}
};

// mehd
struct MovieExtendsHeaderBox {
	uint64_t fragment_duration;

	size_t size(uint8_t version) const {
		return ( 1 == version ) ?  sizeof(uint64_t) : sizeof(uint32_t);
	}
};

// tkhd
struct TrackHeaderBox {
	uint64_t creation_time;
	uint64_t modification_time;
	uint32_t track_ID;
	uint64_t duration;
	int volume;
	std::vector<uint32_t> matrix;
	uint32_t width;
	uint32_t height;

	size_t size(uint8_t version) const {
		switch ( version ) {
			case 1:
				return sizeof(uint64_t) * 3
					+ sizeof(uint32_t) * 15
					+ sizeof(uint16_t) * 4;

			default: // version == 0
				return sizeof(uint32_t) * 18
					+ sizeof(uint16_t) * 4;
		}
	}
};

// mdhd
struct MediaHeaderBox {
	uint64_t creation_time;
	uint64_t modification_time;
	uint32_t timescale;
	uint64_t duration;
	char language[3];

	size_t size(uint8_t version) const {
		switch ( version ) {
			case 1:
				return sizeof(uint64_t) * 3
					+ sizeof(uint32_t)
					+ sizeof(uint16_t) * 2;

			default: // version == 0
				return sizeof(uint32_t) * 4
					+ sizeof(uint16_t) * 2;
		}
	}
};

// hdlr
struct HandlerBox {
	static const uint32_t VIDEO = 0x76696465;
	static const uint32_t AUDIO = 0x736f756e;
	static const uint32_t HINT = 0x68696e74;

	uint32_t handler_type;
	std::string name;

	size_t size() const {
		return sizeof(uint32_t) * 5 + name.size();
	}
};

// vmhd
struct VideoMediaHeaderBox {
	uint16_t graphicsmode;
	uint16_t opcolor[3];

	size_t size() const {
		return sizeof(uint16_t) * 4;
	}
};

// smhd
struct SoundMediaHeaderBox {
	uint16_t balance;

	size_t size() const {
		return sizeof(uint16_t) * 2;
	}
};

// hmhd
struct HintMediaHeaderBox {
	uint16_t maxPDUsize;
	uint16_t avgPDUsize;
	uint32_t maxbitrate;
	uint32_t avgbitrate;

	size_t size() const {
		return sizeof(uint32_t) * 3 + sizeof(uint16_t) * 2;
	}
};

// stsd
namespace SampleDescriptionBox {
	typedef std::pair<BoxHead, std::vector<uint8_t>> SampleDescriptionExtension;

	struct SampleEntry {
		uint32_t data_reference_index;
	};

	struct HintSampleEntry : public SampleEntry {
		std::vector<uint8_t> data;
	};

	struct VisualSampleEntry : public SampleEntry {
		uint16_t pre_defined1;
		uint16_t reserved1;
		uint32_t pre_defined2[3];

		uint16_t width;
		uint16_t height;
		uint32_t horizresolution;
		uint32_t vertresolution;

		uint32_t reserved2;

		uint16_t frame_count;
		char compressorname[32];
		uint16_t depth;

		uint16_t pre_defined3;

		std::vector<std::shared_ptr<SampleDescriptionExtension>> extensions;

		// https://gist.github.com/qiaoxueshi/d9a007a70fedf70b4f9ac88b975e9dc1
		struct AVCDecoderConfigurationRecord {
			uint8_t configuration_version;
			uint8_t avc_profile_indication;
			uint8_t profile_compatibility;
			uint8_t avc_level_indication;
			uint8_t length_size_minus_one;
			uint8_t num_of_sequence_parameter_sets;
			struct SequenceParameterSet {
				uint16_t sequence_parameter_set_length;
				std::vector<uint8_t> sequence_parameter_set_nal_unit;
			};
			std::vector<SequenceParameterSet> sps;
			uint8_t num_of_picture_parameter_sets;
			struct PictureParameterSet {
				uint16_t picture_parameter_set_length;
				std::vector<uint8_t> picture_parameter_set_nal_unit;
			};
			std::vector<SequenceParameterSet> pps;
		};

		// ISO/IEC 14496-15:2014
		// https://stackoverflow.com/questions/32697608/where-can-i-find-hevc-h-265-specs
/*
		struct HEVCDecoderConfigurationRecord {
			unsigned int(8) configurationVersion = 1;
			unsigned int(2) general_profile_space;
			unsigned int(1) general_tier_flag;
			unsigned int(5) general_profile_idc;
			unsigned int(32) general_profile_compatibility_flags;
			unsigned int(48) general_constraint_indicator_flags;
			unsigned int(8) general_level_idc;
			bit(4) reserved = ‘1111’b;
			unsigned int(12) min_spatial_segmentation_idc;
			bit(6) reserved = ‘111111’b;
			unsigned int(2) parallelismType;
			bit(6) reserved = ‘111111’b;
			unsigned int(2) chroma_format_idc;
			bit(5) reserved = ‘11111’b;
			unsigned int(3) bit_depth_luma_minus8;
			bit(5) reserved = ‘11111’b;
			unsigned int(3) bit_depth_chroma_minus8;
			bit(16) avgFrameRate;
			bit(2) constantFrameRate;
			bit(3) numTemporalLayers;
			bit(1) temporalIdNested;
			unsigned int(2) lengthSizeMinusOne;
			unsigned int(8) numOfArrays;
			for (j=0; j < numOfArrays; j++)
			{
				bit(1) array_completeness;
				unsigned int(1) reserved = 0;
				unsigned int(6) NAL_unit_type;
				unsigned int(16) numNalus;
				for (i=0; i< numNalus; i++)
				{
					unsigned int(16) nalUnitLength;
					bit(8*nalUnitLength) nalUnit;
				};
			};
		};
*/
	};

	struct AudioSampleEntry : public SampleEntry {
		uint32_t reserved1[2];

		uint16_t channelcount;
		uint16_t samplesize;

		uint16_t pre_defined1;
		uint16_t reserved2;

		uint32_t samplerate;

		std::vector<std::shared_ptr<SampleDescriptionExtension>> extensions;
	};
};

// stsc
struct SampleToChunkBox {
	struct Entry {
		uint32_t first_chunk;
		uint32_t samples_per_chunk;
		uint32_t sample_description_index;
	};
	std::vector<Entry> entries;

	size_t size() const {
		return sizeof(uint32_t) * ( 1 + entries.size() * 3 );
	}
};

// stts
struct TimeToSampleBox {
	struct Entry {
		uint32_t sample_count;
		uint32_t sample_delta;
	};

	std::vector<Entry> entries;

	size_t size() const {
		return sizeof(uint32_t) * ( 1 + entries.size() * 2 );
	}
};

// ctts
struct CompositionOffsetBox {
	struct Entry {
		uint32_t sample_count;
		uint32_t sample_offset;
	};

	std::vector<Entry> entries;

	size_t size() const {
		return sizeof(uint32_t) * ( 1 + entries.size() * 2 );
	}
};

// stsz
struct SampleSizeBox {
	uint32_t sample_size;
	std::vector<uint32_t> entry_sizes;

	size_t size() const {
		return sizeof(uint32_t) * ( 2 + entry_sizes.size() );
	}
};

// stco
struct ChunkOffsetBox {
	std::vector<uint32_t> chunk_offsets;

	size_t size() const {
		return sizeof(uint32_t) + sizeof(uint32_t) * chunk_offsets.size();
	}
};

// co64
struct ChunkLargeOffsetBox {
	std::vector<uint64_t> chunk_offsets;

	size_t size() const {
		return sizeof(uint32_t) + sizeof(uint64_t) * chunk_offsets.size();
	}
};

// stss
struct SyncSampleBox {
	std::vector<uint32_t> sample_numbers;

	size_t size() const {
		return sizeof(uint32_t) * ( 1 + sample_numbers.size() );
	}
};

// sdtp
struct SampleDependencyTypeBox {
	//aligned(8) class SampleDependencyTypeBox extends FullBox(‘sdtp’, version = 0, 0) { for (i=0; i < sample_count; i++){
	//      unsigned int(2) reserved = 0;
	//      unsigned int(2) sample_depends_on;
	//      unsigned int(2) sample_is_depended_on;
	//      unsigned int(2) sample_has_redundancy;
	//} }
	std::vector<uint8_t> sample_dependencies;

	size_t size() const {
		return sizeof(uint8_t) * sample_dependencies.size();
	}
};

// elst
struct EditListBox {
	struct Entry {
		uint64_t segment_duration;
		int64_t  media_time;
		int16_t media_rate_integer;
		int16_t media_rate_fraction;
	};
	std::vector<Entry> entries;

	size_t size(uint8_t version) const {
		switch ( version ) {
			case 1:
				return sizeof(uint32_t) + ( sizeof(uint64_t) * 2 + sizeof(uint16_t) * 2 ) * entries.size();

			default: // version == 0
				return sizeof(uint32_t) + ( sizeof(uint32_t) * 2 + sizeof(uint16_t) * 2 ) * entries.size();
		}
	}
};

// sidx
struct SegmentIndexBox {
	struct Reference {
		bool reference_type;
		uint32_t reference_size;
		uint32_t subsegment_duration;
		bool contains_SAP;
		uint32_t SAP_delta_time;
	};

	uint32_t reference_id;
	uint32_t timescale;
	uint64_t earliest_presentation_time;
	uint64_t first_offset;
	std::vector<Reference> references;

	size_t size(uint8_t version) const {
		return ( ( 1 == version ) ?
				( sizeof(uint32_t) * 2 + sizeof(uint64_t) * 2 + sizeof(uint16_t) * 2 ) :
				( sizeof(uint32_t) * 4 + sizeof(uint16_t) * 2 ) )
			+ ( sizeof(uint32_t) * 3 ) * references.size();
	}
};

// mfhd
struct MovieFragmentHeaderBox {
	uint32_t sequence_number;

	size_t size() const {
		return sizeof(uint32_t);
	}
};

// trex
struct TrackExtendsBox {
	uint32_t track_ID;
	uint32_t default_sample_description_index;
	uint32_t default_sample_duration;
	uint32_t default_sample_size;
	uint32_t default_sample_flags;
	// default_sample_flags {
	//	bit(6) reserved=0;
	//	unsigned int(2) sample_depends_on;
	//	unsigned int(2) sample_is_depended_on;
	//	unsigned int(2) sample_has_redundancy;
	//	bit(3) sample_padding_value;
	//	bit(1) sample_is_difference_sample; // i.e. when 1 signals a non-key or non-sync sample unsigned
	//	int(16) sample_degradation_priority;
	// }

	size_t size() const {
		return sizeof(uint32_t) * 5;
	}
};

// tfhd
struct TrackFragmentHeaderBox {
	uint32_t track_ID;
	// 0x000001 base-data-offset-present: indicates the presence of the base-data-offset field. This provides an explicit anchor for the data offsets in each track run (see below). If not provided, the base-data- offset for the first track in the movie fragment is the position of the first byte of the enclosing Movie Fragment Box, and for second and subsequent track fragments, the default is the end of the data defined by the preceding fragment. Fragments 'inheriting' their offset in this way must all use the same data-reference (i.e., the data for these tracks must be in the same file).
	// 0x000002 sample-description-index-present: indicates the presence of this field, which over-rides, in this fragment, the default set up in the Track Extends Box.
	// 0x000008 default-sample-duration-present
	// 0x000010 default-sample-size-present
	// 0x000020 default-sample-flags-present
	// 0x010000 duration-is-empty: this indicates that the duration provided in either default-sample-duration,
	// or by the default-duration in the Track Extends Box, is empty, i.e. that there are no samples for this time interval. It is an error to make a presentation that has both edit lists in the Movie Box, and empty- duration fragments.
	uint64_t base_data_offset;
	uint32_t sample_description_index;
	uint32_t default_sample_duration;
	uint32_t default_sample_size;
	uint32_t default_sample_flags;

	size_t size(uint32_t flag) const {
		return sizeof(uint32_t)
			+ ( ( flag & 0x000001 ) ? sizeof(uint64_t) : 0 )
			+ ( ( flag & 0x000002 ) ? sizeof(uint32_t) : 0 )
			+ ( ( flag & 0x000008 ) ? sizeof(uint32_t) : 0 )
			+ ( ( flag & 0x000010 ) ? sizeof(uint32_t) : 0 )
			+ ( ( flag & 0x000020 ) ? sizeof(uint32_t) : 0 );
	}
};

// tfdt
struct TrackFragmentDecodeTimeBox {
	uint64_t decode_time;

	size_t size() const {
		return sizeof(uint64_t);
	}
};

// trun
struct TrackRunBox {
	// 0x000001 data-offset-present.
	// 0x000004 first-sample-flags-present; this over-rides the default flags for the first sample only. This
	// makes it possible to record a group of frames where the first is a key and the rest are difference frames, without supplying explicit flags for every sample. If this flag and field are used, sample-flags shall not be present.
	// 0x000100 sample-duration-present: indicates that each sample has its own duration, otherwise the default is used.
	// 0x000200 sample-size-present: each sample has its own size, otherwise the default is used.
	// 0x000400 sample-flags-present; each sample has its own flags, otherwise the default is used.
	// 0x000800 sample-composition-time-offsets-present; each sample has a composition time offset (e.g. as used for I/P/B video in MPEG).

	struct Sample {
		uint32_t sample_duration;
		uint32_t sample_size;
		uint32_t sample_flags;
		uint32_t sample_composition_time_offset;
	};

	int32_t data_offset;
	uint32_t first_sample_flags;
	std::vector<Sample> samples;

	size_t size(uint32_t flag) const {
		return sizeof(uint32_t)
			+ ( ( flag & 0x000001 ) ? sizeof(uint32_t) : 0 )
			+ ( ( flag & 0x000004 ) ? sizeof(uint32_t) : 0 )
			+ (
					( ( flag & 0x000100 ) ? sizeof(uint32_t) : 0 )
					+ ( ( flag & 0x000200 ) ? sizeof(uint32_t) : 0 )
					+ ( ( flag & 0x000400 ) ? sizeof(uint32_t) : 0 )
					+ ( ( flag & 0x000800 ) ? sizeof(uint32_t) : 0 )
			  ) * samples.size();
	}
};

template <typename T>
struct DataBox {
	T boxdata;
};

struct EmptyBox {
};

// tfra
struct TrackFragmentRandomAccessBox {
	struct Entry {
		uint64_t moof_offset;
		uint32_t traf_number;
		uint32_t trun_number;
		uint32_t sample_number;
	};

	uint32_t track_ID;
	std::map<uint64_t, Entry> entries;
};

// mfro
struct MovieFragmentRandomAccessOffsetBox {
	uint32_t size;
};

// mdat
struct MediaDataBox {
	std::string uri;
	std::map<uint64_t, uint32_t> byte_ranges;
	std::map<uint64_t, uint32_t> chunks;

	size_t size() const;
};

#endif
