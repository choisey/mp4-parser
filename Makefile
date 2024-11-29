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
	$(srcdir)/MP4MultiTrackSegment.cc \
	$(srcdir)/MP4SingleTrackSegment.cc \
	$(srcdir)/MP4SequenceToTime.cc \
	$(srcdir)/MP4MultiTrackSegmentBySequenceNumber.cc \
	$(srcdir)/MP4SingleTrackSegmentBySequenceNumber.cc \
	$(srcdir)/MP4HlsM3u8.cc \
	$(srcdir)/MP4HlsMasterPlaylist.cc \
	$(srcdir)/MP4HlsMediaPlaylist.cc \
	$(srcdir)/MP4DashMpd.cc \
	$(srcdir)/MP4InitializationSegment.cc \
	$(srcdir)/MP4Relocate.cc \
	$(srcdir)/mp4_load.cc \
	$(srcdir)/mp4_save.cc \
	$(srcdir)/mp4_dump.cc \
	$(srcdir)/MP4SetBox.cc \
	$(srcdir)/MP4FragKeyFrame.cc \
	$(srcdir)/mp4_media_stat.cc \
	$(srcdir)/MP4TimeToOffsetLoad.cc \
	$(srcdir)/MP4TimeToOffsetLookup.cc \
	$(srcdir)/mp4.cc \
	$(srcdir)/io_file.cc \
	$(srcdir)/io_local_file.cc \
	$(srcdir)/io_remote_file.cc \
	$(srcdir)/io.cc \
	$(srcdir)/main.cc

DEBUGFLAGS = -g -D _DEBUG
RELEASEFLAGS = -O -D NDEBUG

CXXFLAGS := -Wextra -std=c++11 -I /opt/local/include $(CXXFLAGS) $(DBGFLAGS)
LDFLAGS := -Wl,--no-as-needed -lcurl

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
