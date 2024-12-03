srcdir := .
srcs := $(srcdir)/mp4_object.cc \
	$(srcdir)/mp4_box_types.cc \
	$(srcdir)/mp4_abstract_box.cc \
	$(srcdir)/mp4_container_box.cc \
	$(srcdir)/mp4_file.cc \
	$(srcdir)/mp4_abstract_action.cc \
	$(srcdir)/mp4_playlist.cc \
	$(srcdir)/mp4_codec_conf.cc \
	$(srcdir)/mp4_url_encode.cc \
	$(srcdir)/mp4_segment.cc \
	$(srcdir)/mp4_fragment.cc \
	$(srcdir)/mp4_multi_track_segment.cc \
	$(srcdir)/mp4_single_track_segment.cc \
	$(srcdir)/mp4_sequence_to_time.cc \
	$(srcdir)/mp4_multi_track_segment_by_sequence_number.cc \
	$(srcdir)/mp4_single_track_segment_by_sequence_number.cc \
	$(srcdir)/mp4_hls_m3u8.cc \
	$(srcdir)/mp4_hls_master_playlist.cc \
	$(srcdir)/mp4_hls_media_playlist.cc \
	$(srcdir)/mp4_dash_mpd.cc \
	$(srcdir)/mp4_initialization_segment.cc \
	$(srcdir)/mp4_relocate.cc \
	$(srcdir)/mp4_load.cc \
	$(srcdir)/mp4_save.cc \
	$(srcdir)/mp4_dump.cc \
	$(srcdir)/mp4_set_box.cc \
	$(srcdir)/mp4_frag_key_frame.cc \
	$(srcdir)/mp4_media_stat.cc \
	$(srcdir)/mp4_time_to_offset_load.cc \
	$(srcdir)/mp4_time_to_offset_lookup.cc \
	$(srcdir)/mp4.cc \
	$(srcdir)/io_file.cc \
	$(srcdir)/io_local_file.cc \
	$(srcdir)/io.cc \
	$(srcdir)/main.cc

DEBUGFLAGS = -g -D _DEBUG
RELEASEFLAGS = -O -D NDEBUG

CXXFLAGS := -Wextra -std=c++11 -I /opt/local/include $(CXXFLAGS) $(DBGFLAGS)
LDFLAGS := -Wl,--no-as-needed

BuildDir := .

DebugDir := $(BuildDir)/Debug
DebugOutput := $(BuildDir)/mp4d
DebugObjs := $(patsubst %.cc,$(DebugDir)/%.o,$(notdir $(srcs)))

ReleaseDir := $(BuildDir)/Release
ReleaseOutput := $(BuildDir)/mp4
ReleaseObjs := $(patsubst %.cc,$(ReleaseDir)/%.o,$(notdir $(srcs)))

#
# objects
#

$(DebugDir)/%.o: $(srcdir)/%.cc
	mkdir -p $(DebugDir)
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c -o $@ $<

$(ReleaseDir)/%.o: $(srcdir)/%.cc
	mkdir -p $(ReleaseDir)
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -c -o $@ $<

#
# output
#
 
all: $(DebugOutput) $(ReleaseOutput)

$(DebugOutput): $(DebugObjs)
	$(CXX) $(LDFLAGS) -o $@ $(DebugObjs)

$(ReleaseOutput): $(ReleaseObjs)
	$(CXX) $(LDFLAGS) -o $@ $(ReleaseObjs)

#
# make clean
#

.PHONY: install clean

clean:
	rm -rf $(DebugDir) $(ReleaseDir)
