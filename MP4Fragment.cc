/*
 * Copyright (c) Seungyeob Choi
 */

#include "MP4Fragment.h"
#include "MP4Relocate.h"
#include "mp4.h"
#include <assert.h>

#define TRUN_DATA_OFFSET_PRESENT	0x000001
#define TRUN_FIRST_SAMPLE_FLAGS_PRESENT	0x000004
#define TRUN_SAMPLE_DURATION_PRESENT	0x000100
#define TRUN_SAMPLE_SIZE_PRESENT	0x000200
#define TRUN_SAMPLE_FLAGS_PRESENT	0x000400
#define TRUN_SAMPLE_COMPOSITION_TIME_OFFSETS_PRESENT 0x000800

// MP4FragmentVisitor

MP4Fragment::MP4FragmentVisitor::MP4FragmentVisitor(uint64_t decode_time)
	: _decode_time(decode_time)
{
}

MP4Fragment::MP4FragmentVisitor::~MP4FragmentVisitor()
{
}

void MP4Fragment::MP4FragmentVisitor::visit(BoxHead& head, std::vector<std::shared_ptr<mp4_abstract_box>>& boxes)
{
	assert( MP4FILE == head.boxtype );

	std::vector<std::shared_ptr<mp4_abstract_box>> fmp4_boxes;

	// sidx
	std::shared_ptr<mp4_concrete_box<SegmentIndexBox>> sidx = std::make_shared<mp4_concrete_box<SegmentIndexBox>>(
			BoxHead { 0, 0, 0, SIDX, 0, 0 }
			);
	sidx->data().reference_id = 1;
	sidx->data().timescale = 0;
	sidx->data().earliest_presentation_time = _decode_time;
	sidx->data().first_offset = 0;

	// moof
	std::shared_ptr<mp4_container_box> moof = std::make_shared<mp4_container_box>(
			BoxHead { 0, 0, 0, MOOF, 0, 0 }
			);

	// mdat
	std::shared_ptr<mp4_abstract_box> mdat;

	uint32_t duration = 0;
	uint32_t reference_size = 0;

	// sidx.timescale := mdhd.timescale
	// sidx.references.reference_size := moof + mdat
	// sidx.references.subsegment_duration := stts.sample_count * stts.sample_delta

	for ( auto child: boxes ) {
		switch ( child->head().boxtype ) {
			case MOOV:
				{
					const auto& mvhd = select<MovieHeaderBox>(child);
					assert( 1 == mvhd.size() );
					if ( 1 != mvhd.size() ) continue;

					std::shared_ptr<mp4_concrete_box<MovieFragmentHeaderBox>> mfhd = std::make_shared<mp4_concrete_box<MovieFragmentHeaderBox>>(
							BoxHead { 0, 0, 0, MFHD, 0, 0 }
							);
					mfhd->data().sequence_number = 1;
					moof->addChild(mfhd);

					// assert in _DEBUG
					// fmp4 with multiple moof in NDEBUG
					assert( 1 == select(child, TRAK).size() );

					for ( auto trak: select(child, TRAK) ) {
						// tkhd, mdhd
						const auto& tkhd = select<TrackHeaderBox>(trak);
						const auto& mdhd = select<MediaHeaderBox>(trak);
						assert( 1 == tkhd.size() );
						assert( 1 == mdhd.size() );

						// stts, stsz, stss, ctts
						const auto& stts = select<TimeToSampleBox>(trak);
						const auto& stsz = select<SampleSizeBox>(trak);
						const auto& stss = select<SyncSampleBox>(trak);
						const auto& ctts = select<CompositionOffsetBox>(trak);

						assert( 1 == stsz.size() );
						assert( 1 == stts.size() );
						//assert( 1 == stss.size() );

						if ( 1 != tkhd.size()
								|| 1 != mdhd.size()
								|| 1 != stts.size()
								|| 1 != stsz.size() ) continue;

						sidx->data().timescale = mdhd[0]->data().timescale;

						// tfhd
						std::shared_ptr<mp4_concrete_box<TrackFragmentHeaderBox>> tfhd = std::make_shared<mp4_concrete_box<TrackFragmentHeaderBox>>(
								BoxHead { 0, 0, 0, TFHD, 0, 0 }
								);
						tfhd->head().flag = 0;
						tfhd->data().track_ID = tkhd[0]->data().track_ID;;

						// tfdt
						std::shared_ptr<mp4_concrete_box<TrackFragmentDecodeTimeBox>> tfdt = std::make_shared<mp4_concrete_box<TrackFragmentDecodeTimeBox>>(
								BoxHead { 0, 0, 0, TFDT, 0, 0 }
								);
						tfdt->data().decode_time = _decode_time;

						// trun
						std::shared_ptr<mp4_concrete_box<TrackRunBox>> trun = std::make_shared<mp4_concrete_box<TrackRunBox>>(
								BoxHead { 0, 0, 0, TRUN, 0, 0 }
								);
						trun->head().flag =
							TRUN_DATA_OFFSET_PRESENT
							| TRUN_SAMPLE_DURATION_PRESENT
							| TRUN_SAMPLE_SIZE_PRESENT;
						size_t trun_sample_fields = 2;

						// construct trun->{ ... } and set sample_duration using stts
						for ( auto s: stts[0]->data().entries ) {
							for ( uint32_t i = 0; i < s.sample_count; i++ ) {
								trun->data().samples.push_back( TrackRunBox::Sample {
										s.sample_delta, 0, 0, 0 } );
								duration += (uint64_t) s.sample_count * s.sample_delta;
							}
						}

						// set trun->sample_size
						if ( 0 == stsz[0]->data().sample_size ) {
							assert( trun->data().samples.size() == stsz[0]->data().entry_sizes.size() );

							auto szi = stsz[0]->data().entry_sizes.begin();
							for ( auto& s: trun->data().samples ) {
								s.sample_size = *szi++;
							}

							assert( szi == stsz[0]->data().entry_sizes.end() );
						}
						else {
							for ( auto& s: trun->data().samples ) {
								s.sample_size = stsz[0]->data().sample_size;
							}
						}

						// set trun->sample_flags in case stss is present
						if ( !stss.empty() ) {
							trun->head().flag |= TRUN_SAMPLE_FLAGS_PRESENT;
							trun_sample_fields += 1;

							for ( auto& s: trun->data().samples ) {
								s.sample_flags = 0x1010000;
							}

							for ( auto s: stss[0]->data().sample_numbers ) {
								trun->data().samples[ s - 1 ].sample_flags = 0x2000000;
							}
						}

						// set trun->sample_composition_time_offset in case ctts is present
						if ( !ctts.empty() ) {
							trun->head().flag |= TRUN_SAMPLE_COMPOSITION_TIME_OFFSETS_PRESENT;
							trun_sample_fields += 1;

							auto cti = ctts[0]->data().entries.begin();
							uint32_t sample_count = 0;
							uint32_t sample_offset = 0;
							for ( auto& s: trun->data().samples ) {
								if ( sample_count < 1 ) {
									assert( cti != ctts[0]->data().entries.end() );
									sample_count = (*cti).sample_count;
									sample_offset = (*cti).sample_offset;
									cti++;
								}
								s.sample_composition_time_offset = sample_offset;
								--sample_count;
							}

							assert( cti == ctts[0]->data().entries.end() );
						}

						std::shared_ptr<mp4_container_box> traf = std::make_shared<mp4_container_box>(
								BoxHead { 0, 0, 0, TRAF, 0, 0 }
								);

						traf->addChild(tfhd);
						traf->addChild(tfdt);
						traf->addChild(trun);
						moof->addChild(traf);

						Relocate(moof);

						assert( sizeof(uint32_t) * 2 == moof->head().boxheadsize );
						assert( sizeof(uint32_t) * 4 == mfhd->head().boxheadsize + mfhd->data().size() );
						assert( sizeof(uint32_t) * 2 == traf->head().boxheadsize );
						assert( sizeof(uint32_t) * 4 == tfhd->head().boxheadsize + tfhd->data().size(tfhd->head().flag) );
						assert( sizeof(uint32_t) * 4 == tfdt->head().boxheadsize + tfdt->data().size() );
						assert( sizeof(uint32_t) * 5 + trun->data().samples.size() * sizeof(uint32_t) * trun_sample_fields
								== trun->head().boxheadsize + trun->data().size(trun->head().flag) );

						reference_size += ( moof->head().boxheadsize
								+ mfhd->head().boxheadsize + mfhd->data().size()
								+ traf->head().boxheadsize
								+ tfhd->head().boxheadsize + tfhd->data().size(tfhd->head().flag)
								+ tfdt->head().boxheadsize + tfdt->data().size()
								+ trun->head().boxheadsize + trun->data().size(trun->head().flag)
								);

						trun->data().data_offset = reference_size
							+ sizeof(uint32_t) * 2; // mdat header
					}

					break;
				}

			case MDAT:
				assert( child->istype( typeid(MediaDataBox) ) );
				reference_size += (
						sizeof(uint32_t) * 2
						+  std::static_pointer_cast<mp4_concrete_box<MediaDataBox>>(child)->data().size()
						);
				mdat = child;
				break;
		}
	}

	sidx->data().references.push_back(
			SegmentIndexBox::Reference {
			//	bool reference_type;
			//	uint32_t reference_size;
			//	uint32_t subsegment_duration;
			//	bool contains_SAP;
			//	uint32_t SAP_delta_time;
			0, reference_size, duration, 1, 268435456
			});

	fmp4_boxes.push_back(sidx);
	fmp4_boxes.push_back(moof);
	if ( mdat ) {
		fmp4_boxes.push_back(mdat);
	}
	boxes = fmp4_boxes;
}

// MP4Fragment

MP4Fragment::MP4Fragment(uint64_t time)
	: _decode_time(time)
{
}

MP4Fragment::~MP4Fragment()
{
}

void MP4Fragment::execute(std::shared_ptr<mp4_abstract_box> mp4)
{
	assert( MP4FILE == mp4->head().boxtype );
	MP4FragmentVisitor v(_decode_time);
	mp4->accept(&v);
}
