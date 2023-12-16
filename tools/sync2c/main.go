package main

import (
	"bufio"
	"flag"
	"log"
	"os"
	"strconv"
	"strings"
	"text/template"
)

var printHelp bool

const (
	TrkCtrl = -2 // control key
	TrkEnd  = -1 // last frame (sentinel element)

	TrkStep      = 1 // set constant value
	TrkLinear    = 2 // lerp to the next value
	TrkSmooth    = 3 // smooth curve to the next value
	TrkQuadratic = 4 // lerp with quadratic factor
	TrkTrigger   = 5 // count down (with every frame) from given number
	TrkEvent     = 6 // like step but value is delivered only once
)

const (
	FramesPerRow = 3 // Assuming constant BPM of 125
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

func parseFrame(token string) (frame int64, err error) {
	if token[0] == '$' {
		if len(token) != 5 {
			err = &parseError{"key must be 4 hex digit protracker song position"}
			return
		}

		frame, err = strconv.ParseInt(token[1:], 16, 16)
		if err != nil {
			return
		}

		if frame&0xC0 != 0 {
			err = &parseError{"not a valid pattern row"}
			return
		}

		frame = frame&63 | (frame>>2)&-64
		frame *= FramesPerRow
	} else {
		var f float64
		f, err = strconv.ParseFloat(token, 64)
		if err == nil {
			frame = int64(f * 50.0)
		}
	}

	if prevFrame >= 0 {
		delta := frame - prevFrame
		if delta < 0 {
			return 0, &parseError{
				"frame numbers must be specified in ascending order"}
		}
		prevFrame = frame
		frame = delta
	} else {
		prevFrame = frame
	}

	return frame, err
}

func parseValue(token string) (value int64, err error) {
	return strconv.ParseInt(token, 0, 16)
}

var prevFrame int64

func parseTrack(tokens []string, track *Track) (err error) {
	var frame, value int64

	if track == nil {
		return &parseError{"key frame outside of track"}
	}

	if frame, err = parseFrame(tokens[0]); err != nil {
		return err
	}
	if len(tokens) < 2 {
		return &parseError{"missing value"}
	}
	if value, err = parseValue(tokens[1]); err != nil {
		return err
	}

	if len(tokens) == 3 && tokens[2][0] == '!' {
		typ := tokens[2][1:]
		if typ == "step" {
			track.AddItem(TrkCtrl, TrkStep)
		} else if typ == "linear" {
			track.AddItem(TrkCtrl, TrkLinear)
		} else if typ == "smooth" {
			track.AddItem(TrkCtrl, TrkSmooth)
		} else if typ == "quadratic" {
			track.AddItem(TrkCtrl, TrkQuadratic)
		} else if typ == "trigger" {
			track.AddItem(TrkCtrl, TrkTrigger)
		} else if typ == "event" {
			track.AddItem(TrkCtrl, TrkEvent)
		} else {
			return &parseError{"unknown track type"}
		}
	}

	track.AddItem(int(frame), int(value))

	return nil
}

func parseSyncFile(path string) []Track {
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
			prevFrame = -1
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

		if err = parseTrack(tokens, track); err != nil {
			log.Println(origLine)
			log.Fatalf("Parse error at line %d: %s", num, err)
		}
	}

	if track != nil {
		log.Fatalf("Last track '%s' has not been closed!", track.RawName)
	}

	return tracks
}

var tracksTemplate = `
{{- range . }}
TrackT {{ .Name }} = {
  .curr = NULL,
  .next = NULL,
  .type = 0,
  .interval = 0,
  .delta = 0,
  .pending = false,
  .data = {
{{- range .Items }}
    { {{ .Key }}, {{ .Value }} },
{{- end }}
  }
};
{{ end }}
static TrackT *AllTracks[] = {
{{- range . }}
  &{{ .Name }},
{{- end }}
  NULL
};
`

func exportTracks(tracks []Track) {
	t, err := template.New("export").Parse(tracksTemplate)
	if err != nil {
		log.Fatal(err)
	}

	err = t.Execute(os.Stdout, tracks)
	if err != nil {
		log.Fatal(err)
	}

	return
}

func init() {
	flag.BoolVar(&printHelp, "help", false, "print help message and exit")
}

func main() {
	flag.Parse()

	if len(flag.Args()) < 1 || printHelp {
		flag.PrintDefaults()
		os.Exit(1)
	}

	exportTracks(parseSyncFile(flag.Arg(0)))
}
