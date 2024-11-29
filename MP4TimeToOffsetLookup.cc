/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4TimeToOffsetLookup.h"
#include "MP4.h"
#include <assert.h>

MP4TimeToOffsetLookup::MP4TimeToOffsetLookup(uint32_t track_id, uint64_t time)
	: _track_id(track_id)
	, _time(time)
{
}

MP4TimeToOffsetLookup::~MP4TimeToOffsetLookup()
{
}

void MP4TimeToOffsetLookup::execute(std::shared_ptr<MP4AbstractBox> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );

	//	+------+       +------+
	//	| MFRA | --+-- | TFRA |
	//	+------+   |   +------+
	//		   |
	//		   |   +------+
	//		   +-- | TFRA |
	//		   |   +------+
	//		   |
	//
	//			...
	//
	//		   |
	//		   |   +------+
	//		   +-- | MFRO |
	//		       +------+

	// mfra

	const auto& mfra = select(mp4, MFRA);
	assert( 1 == mfra.size() );
	if ( mfra.empty() ) return;

	// tfra

	std::map<uint64_t, uint64_t> moof_offset_to_time;
	for ( const auto& tfra: select<TrackFragmentRandomAccessBox>(mfra[0]) ) {
		for ( auto e: tfra->data().entries ) {
			moof_offset_to_time[e.second.moof_offset] = e.first;
		}
	}

	// 5/16/2018
	// STREAM-1372
	// INC0465324
	// Last smooth segment fails, because there is no upper bound in the map.
	moof_offset_to_time[ mfra[0]->head().offset ] = MAX_64BIT;

	for ( const auto& tfra: select<TrackFragmentRandomAccessBox>(mfra[0]) ) {
		if ( tfra->data().track_ID == _track_id ) {
			std::map<uint64_t, uint64_t> time_to_moof_offset;
			for ( auto e: tfra->data().entries ) {
				time_to_moof_offset[e.first] = e.second.moof_offset;
			}

			time_to_moof_offset[MAX_64BIT] = mfra[0]->head().offset;

			auto lb = time_to_moof_offset.lower_bound(_time);
			//auto ub = time_to_moof_offset.upper_bound(_time);
			auto ub = moof_offset_to_time.upper_bound(lb->second);

			assert( time_to_moof_offset.end() != lb );
			assert( moof_offset_to_time.end() != ub );

			if ( lb->second < ub->first ) {
				printf("%lu-%lu/%lu\n", lb->second, ub->first - 1, ub->first - lb->second);
			}

			break;
		}
	}
}
