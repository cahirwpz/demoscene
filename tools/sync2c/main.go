package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
)

const (
	TrkCtrl    = -2 // control key
	TrkEnd     = -1 // last frame (sentinel element)
	TrkRamp    = 1  // set constant value
	TrkLinear  = 2  // lerp to the next value
	TrkSmooth  = 3  // smooth curve to the next value
	TrkSpline  = 4  // hermite spline
	TrkTrigger = 5  // count down (with every frame) from given number
	TrkEvent   = 6  // like ramp but value is delivered only once
)

const (
	FramesPerRow = 6 // Assuming constant BPM of 125
)

type TrackItem struct {
	Key   int
	Value int
}

type Track struct {
	RawName string
	Items   []TrackItem
	First   *TrackItem
	Last    *TrackItem
}

func (t Track) Name() string {
	return strings.ReplaceAll(t.RawName, ".", "_")
}

func (t *Track) AddItem(frame, value int) {
	item := TrackItem{frame, value}
	t.Items = append(t.Items, item)
	if frame >= 0 {
		if t.First == nil {
			t.First = &item
		}
		t.Last = &item
	}
}

func (t Track) Length() int {
	return t.Last.Key - t.First.Key
}

type parseError struct {
	cause string
}

func (e *parseError) Error() string {
	return e.cause
}

func ParseTrack(tokens []string, track *Track) error {
	if track == nil {
		return &parseError{"key frame outside of track"}
	}

	key := tokens[0]

	if key[0] == '!' {
		if key == "!ramp" {
			track.AddItem(TrkCtrl, TrkRamp)
		} else if key == "!linear" {
			track.AddItem(TrkCtrl, TrkLinear)
		} else if key == "!smooth" {
			track.AddItem(TrkCtrl, TrkSmooth)
		} else if key == "!spline" {
			track.AddItem(TrkCtrl, TrkSpline)
		} else if key == "!trigger" {
			track.AddItem(TrkCtrl, TrkTrigger)
		} else if key == "!event" {
			track.AddItem(TrkCtrl, TrkEvent)
		} else {
			return &parseError{"unknown track type"}
		}
	} else if key[0] == '$' {
		if len(tokens) != 2 {
			return &parseError{"missing value"}
		}
		if len(key) != 5 {
			return &parseError{"key must be 4 hex digit protracker song position"}
		}
		var frame, value int64
		var err error
		if frame, err = strconv.ParseInt(key[1:], 16, 16); err != nil {
			return err
		}
		if value, err = strconv.ParseInt(tokens[1], 10, 16); err != nil {
			return err
		}
		frame *= FramesPerRow
		if track.First != nil && track.First.Key > int(frame) {
			return &parseError{"frame numbers must be specified in ascending order"}
		}
		track.AddItem(int(frame), int(value))
	} else {
		return &parseError{"unknown key or command"}
	}

	return nil
}

func ParseSyncFile(path string) []Track {
	file, err := os.Open(path)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)

	var tracks []Track
	var track *Track = nil
	num := 0

	for scanner.Scan() {
		num++
		line := scanner.Text()
		origLine := string(line)

		// Strip comment
		if idx := strings.IndexByte(line, '#'); idx >= 0 {
			line = line[0:idx]
		}

		// Detect and skip empty lines
		line = strings.Trim(line, " ")
		if line == "" {
			continue
		}

		// Split track data into tokens
		tokens := strings.Fields(line)

		// Create new track
		if tokens[0] == "@track" {
			track = &Track{RawName: tokens[1]}
			continue
		}

		// Close a track
		if tokens[0] == "@end" {
			if track.First == nil {
				log.Fatalf("Track '%s' has no frames!", track.RawName)
			}
			track.AddItem(TrkEnd, 0)
			tracks = append(tracks, *track)
			track = nil
			continue
		}

		if err = ParseTrack(tokens, track); err != nil {
			log.Println(origLine)
			log.Fatalf("Parse error at line %d: %s", num, err)
		}
	}

	if track != nil {
		log.Fatalf("Last track '%s' has not been closed!", track.RawName)
	}

	return tracks
}

func main() {
	tracks := ParseSyncFile("sushiboyz.sync")
	for _, t := range tracks {
		fmt.Printf("static TrackT %s_trk = {\n", t.Name())
		fmt.Println("  .type = TRACK_LINEAR,")
		fmt.Printf("  .name = \"%s\",\n", t.RawName)
		fmt.Println("  .data = {")
		for _, i := range t.Items {
			fmt.Printf("    {%d, %d},\n", i.Key, i.Value)
		}
		fmt.Println("  }")
		fmt.Println("};\n")
	}

	fmt.Printf("TrackT tracks[] = {\n")
	for _, t := range tracks {
		fmt.Printf("  &%s,\n", t.Name())
	}
	fmt.Println("  NULL")
	fmt.Println("};\n")
}
