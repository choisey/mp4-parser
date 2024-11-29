srcdir := .
srcs := $(srcdir)/MP4Obj.cc \
	$(srcdir)/MP4BoxTypes.cc \
	$(srcdir)/MP4AbstractBox.cc \
	$(srcdir)/MP4ContainerBox.cc \
	$(srcdir)/MP4File.cc \
	$(srcdir)/MP4AbstractAction.cc \
	$(srcdir)/MP4Playlist.cc \
	$(srcdir)/MP4CodecConf.cc \
	$(srcdir)/MP4URLEncode.cc \
	$(srcdir)/MP4Segment.cc \
	$(srcdir)/MP4Fragment.cc \
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
	$(srcdir)/MP4Load.cc \
	$(srcdir)/MP4Save.cc \
	$(srcdir)/MP4Dump.cc \
	$(srcdir)/MP4SetBox.cc \
	$(srcdir)/MP4FragKeyFrame.cc \
	$(srcdir)/MP4MediaStat.cc \
	$(srcdir)/MP4TimeToOffsetLoad.cc \
	$(srcdir)/MP4TimeToOffsetLookup.cc \
	$(srcdir)/MP4.cc \
	$(srcdir)/DCP.cc \
	$(srcdir)/File.cc \
	$(srcdir)/LocalFile.cc \
	$(srcdir)/RemoteFile.cc \
	$(srcdir)/IO.cc \
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
