package main

import (
	"bytes"
	"flag"
	"fmt"
	"html/template"
	"log"
	"math"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"github.com/joshvarga/svgparser"
)

const (
	svgTemplate = `static GreetsT {{ .Name }} = {
  .curr = NULL,
  .n = 0,
  .x = 0,
  .y = 0,
  .origin_x = {{ .Origin.X }},
  .origin_y = {{ .Origin.Y }},
  .delay = {{ .Delay }},
  .data = {
    {{- range .Data }}
    {{ .Length }},
    {{- range .DeltaPoints }}{{ .X }},{{.Y}},{{- end }}
    {{- end }}
    -1
  }
};
`
)

// A SVG data container for a single file
type SvgData struct {
	Data   []GeometricData // Individual SVG elements data
	Origin Point           // Origin point for whole SVG
	Delay  int             // Delay in frames
	Name   string          // Name of the data (file name)
}

// A SVG data for individual SVG element eg. line, polyline
type GeometricData struct {
	Length      int     // Length of the points slice
	Points      []Point // Standard points representation
	DeltaPoints []Point // Delta encoded points representation
}

type Point struct {
	X int
	Y int
}

func Add(p1, p2 Point) (p Point) {
	return Point{p1.X + p2.X, p1.Y + p2.Y}
}

func Sub(p1, p2 Point) (p Point) {
	return Point{p1.X - p2.X, p1.Y - p2.Y}
}

// parseCoordinate parses string float representation to integer eg. "123.45" -> 123
func parseCoordinate(coord string) (int, error) {
	num, err := strconv.ParseFloat(coord, 32)
	if err != nil {
		return 0, err
	}
	return int(math.Round(num)), nil
}

// parseLine parses SVG line element and returns slice (pair) of Points
// line element has explicitly defined attributes x1, y1, x2, y2
func parseLine(el *svgparser.Element) []Point {
	X1, err := parseCoordinate(el.Attributes["x1"])
	if err != nil {
		log.Fatalf("Error parsing line coord x1 - value: %s\n",
			el.Attributes["x1"])
	}
	Y1, err := parseCoordinate(el.Attributes["y1"])
	if err != nil {
		log.Fatalf("Error parsing coordinates y1 - value: %s\n",
			el.Attributes["y1"])
	}
	X2, err := parseCoordinate(el.Attributes["x2"])
	if err != nil {
		log.Fatalf("Error parsing coordinates x2 - value: %s\n",
			el.Attributes["x2"])
	}
	Y2, err := parseCoordinate(el.Attributes["y2"])
	if err != nil {
		log.Fatalf("Error parsing coordinates y2 - value: %s\n",
			el.Attributes["y2"])
	}
	return []Point{{X1, Y1}, {X2, Y2}}
}

// parsePolyline parses polyline SVG element and returns slice of elements
// polyline holds points data in "x1,y1 x2,y2 ... xn,yn" format
func parsePolyLine(el *svgparser.Element) []Point {
	pointsString := el.Attributes["points"]
	space := regexp.MustCompile(`\s+`)
	trimmedPoints := space.ReplaceAllString(pointsString, " ")
	points := strings.Split(trimmedPoints, " ")
	var parsedPoints = make([]Point, 0, len(points))
	for _, point := range points {
		if point == "" {
			continue
		}
		coords := strings.Split(point, ",")
		x, err := parseCoordinate(strings.TrimSpace(coords[0]))
		if err != nil {
			log.Fatalf("Error parsing coordinate x in point: '%v'\n", point)
		}
		y, err := parseCoordinate(coords[1])
		if err != nil {
			log.Fatalf("Error parsing coordinate y in point: '%v'\n", point)
		}
		parsedPoint := Point{x, y}
		parsedPoints = append(parsedPoints, parsedPoint)
	}
	return parsedPoints
}

// calcOrigin calculates coordinates of the point which is
// in the middle of the SVG bounding box from the top-left corner of the screen
func (sd *SvgData) calcOrigin() {
	minPoint := sd.getMinPoint()
	maxPoint := sd.getMaxPoint()
	midPoint := Point{
		(maxPoint.X - minPoint.X) / 2,
		(maxPoint.Y - minPoint.Y) / 2,
	}
	sd.Origin = Add(minPoint, midPoint)
}

func (sd *SvgData) calcDataWithOffset() {
	for idx, gData := range sd.Data {
		sd.Data[idx].DeltaPoints = gData.getWithOffsetAndDelta(sd.Origin)
	}
}

// getMinPoint calculates top-left corner of the SVG bounding box,
// the closest point to the top-left corner of the screen.
func (sd *SvgData) getMinPoint() Point {
	var minPoint = sd.Data[0].Points[0]
	for _, gData := range sd.Data {
		for _, point := range gData.Points {
			if point.X < minPoint.X {
				minPoint.X = point.X
			}
			if point.Y < minPoint.Y {
				minPoint.Y = point.Y
			}
		}
	}
	return minPoint
}

// getMaxPoint calculates bottom-right corner of the SVG bounding box,
// the farthest point to the top-left corner of the screen.
func (sd *SvgData) getMaxPoint() Point {
	var maxPoint = sd.Data[0].Points[0]
	for _, gData := range sd.Data {
		for _, point := range gData.Points {
			if point.X > maxPoint.X {
				maxPoint.X = point.X
			}
			if point.Y > maxPoint.Y {
				maxPoint.Y = point.Y
			}
		}
	}
	return maxPoint
}

// Export returns string data of the SvgData structure
func (sd *SvgData) Export() (output string, err error) {
	t, err := template.New("svg").Parse(svgTemplate)
	if err != nil {
		return
	}

	file, err := os.Create(outputPath)
	if err != nil {
		return "", err
	}
	defer file.Close()

	var data bytes.Buffer
	err = t.Execute(&data, sd)
	if err != nil {
		return "", err
	}
	return data.String(), nil
}

// getWithOffsetAndDelta returns points with offset compressed with delta compression,
func (gd *GeometricData) getWithOffsetAndDelta(offset Point) []Point {
	inputPoints := gd.getPointsWithOffset(offset)
	deltaPoints := []Point{inputPoints[0]}
	for index, point := range inputPoints {
		if index != 0 {
			lastPoint := inputPoints[index-1]
			deltaPoints = append(deltaPoints, Sub(point, lastPoint))
		}
	}
	return deltaPoints
}

// getPointsWithOffset returns points translated by the offset
func (gd *GeometricData) getPointsWithOffset(offset Point) []Point {
	var withOffset []Point
	for _, point := range gd.Points {
		withOffset = append(withOffset, Sub(point, offset))
	}
	return withOffset
}

// handleFile returns file data as converted string
func handleFile(file *os.File, name string, delay int) string {
	svgData := SvgData{[]GeometricData{}, Point{0, 0}, delay, name}

	element, err := svgparser.Parse(file, false)
	if err != nil {
		log.Fatal(err)
	}
	elements := element.Children

	for _, element := range elements {
		elementType := element.Name
		switch elementType {
		case "polyline":
			parsedPoints := parsePolyLine(element)
			svgData.Data = append(svgData.Data,
				GeometricData{len(parsedPoints), parsedPoints, []Point{}})
		case "line":
			parsedPoints := parseLine(element)
			svgData.Data = append(svgData.Data,
				GeometricData{len(parsedPoints), parsedPoints, []Point{}})
		case "path":
			if verbose {
				fmt.Println("Found path element, geometry won't be parsed")
			}
		default:
			if verbose {
				fmt.Printf("[WARN] Parsed element %s\n", element.Name)
			}
		}
	}
	svgData.calcOrigin()
	svgData.calcDataWithOffset()
	data, err := svgData.Export()
	if err != nil {
		log.Fatal(err)
	}
	return data
}

const defaultFileName = "data.c"

var outputPath, structName string
var verbose, printHelp bool
var delayValue int

func init() {
	flag.StringVar(&outputPath, "o", defaultFileName, "Output filename with processed data")
	flag.StringVar(&structName, "name", "", "Custom structure name")
	flag.BoolVar(&verbose, "v", false, "Output verbose logs")
	flag.IntVar(&delayValue, "delay", 0, "Delay in frames")
	flag.BoolVar(&printHelp, "help", false, "Prints this message")
}

func main() {

	flag.Parse()

	if printHelp || len(flag.Args()) < 1 || len(outputPath) == 0 {
		flag.PrintDefaults()
		os.Exit(1)
	}

	var exportString string

	if len(flag.Args()) != 1 {
		log.Fatalln("svg2c handles only single file conversion at once")
	}

	fileName := flag.Args()[0]

	file, err := os.Open(fileName)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	fileExt := filepath.Ext(fileName)
	fileBase := filepath.Base(fileName)
	if len(structName) == 0 {
		structName = strings.TrimSuffix(fileBase, fileExt)
		structName = strings.Replace(structName, "-", "_", -1)
	}

	if fileExt == ".svg" {
		if verbose {
			fmt.Println("Converting file:", fileName)
		}
		exportString += handleFile(file, structName, delayValue)
	}

	if len(exportString) == 0 {
		os.Exit(0)
	}

	outFile, err := os.Create(outputPath)
	if err != nil {
		log.Fatal(err)
	}
	defer outFile.Close()
	outFile.WriteString(exportString)
	if err != nil {
		log.Fatal(err)
	}
}
