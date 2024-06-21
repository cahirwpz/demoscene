/* all indices are offsets in bytes from the beginning of data array */

static short _{{ .Name }}_data[] = {
  /* vertices: [flags orig_x orig_y orig_z x y z] */
  /* offset: 0, count: {{ len .Vertices }} */
  {{- range .Vertices }}
  {{ range . }}{{ . }}, {{ end -}}
{{ end }} 

  /* edges: [flags vertex-index-0 vertex-index-1] */
  /* offset: {{ .EdgeOffset }}, count: {{ len .Edges }} */
  {{- range .Edges }}
  {{ .Flags }}, {{ range .Point }}{{ . }}, {{ end -}}
{{- end }}

  /* faces: [face-normal{x y z} {flags:8 material:8} #indices | {vertex-index edge-index}...] */
  /* lines/points: [{flags:8 material:8} #indices | vertex-index...] */
  /* offset: {{ .FaceDataOffset }}, count: {{ .FaceDataCount }} */
  {{- range .Faces }}
  {{range .Normal }}{{ . }}, {{ end -}}{{.Material}}, {{.Count}}, {{ range .Indices }}{{ . }}, {{ end -}}
{{- end}}

  /* vertex-groups: [vertex-index... 0] */
  {{- range $obj := .Objects }}

  {{- range .Groups }}
  /* {{ $obj.Name }}:{{ .Name }}, offset: {{ .Vertex.Offset }} */
  {{ range .Vertex.Indices }}{{ . }}, {{ end -}} 0,
{{- end}}
{{- end}}
  /* end */
  0,

  /* edge-groups: [edge-index... 0] */
  {{- range $obj := .Objects }}

  {{- range .Groups }}
  {{- if len .Edge.Indices }}
  /* {{ $obj.Name }}:{{ .Name }} offset: {{ .Edge.Offset }} */
  {{ range .Edge.Indices }}{{ . }}, {{ end -}} 0,
  {{- end }}
{{- end}}
{{- end}}
  /* end */
  0,

  /* face-groups: [face-index... 0] */
  {{- range $obj := .Objects }}

  {{- range .Groups }}
  /* {{ $obj.Name }}:{{ .Name }} offset: {{ .Face.Offset }} */
  {{ range .Face.Indices }}{{ . }}, {{ end -}} 0,
{{- end }}
{{- end }}
  /* end */
  0,

  /* object: [#groups [vertices-offset edge-offset faces-offset]] */
  /* offset: {{ .ObjectDataOffset }} */
  {{- range $obj := .Objects }}

  /* object: {{ $obj.Name }} */
  {{ len .Groups }},
  {{- range .Groups }}
    /* group: {{ .Name }} */
    {{ .Vertex.Offset }}, {{ .Edge.Offset }}, {{ .Face.Offset }},
{{- end}}
{{- end}}

  /* end */
  0,
};

{{ range $obj := .Objects }}
#define obj_{{ $obj.Name }} {{ $obj.Offset }}
{{- range $grp := $obj.Groups }}
#define grp_{{ $obj.Name }}_{{ $grp.Name }} {{ $grp.Offset }}
{{- end}}
{{- end}}
{{ range .Materials }}
#define mtl_{{ $.Name }}_{{ .Name }} {{ .Index }}
{{- end}}

Mesh3D {{ .Name }} = {
  .vertices = {{ len .Vertices }},
  .edges = {{ len .Edges }},
  .faces = {{ len .Faces }},
  .materials = {{ len .Materials }},
  .data = _{{ .Name }}_data,
  .vertexGroups = (void *)_{{ .Name }}_data + {{ .VertexGroupDataOffset }},
  .edgeGroups = (void *)_{{ .Name }}_data + {{ .EdgeGroupDataOffset }},
  .faceGroups = (void *)_{{ .Name }}_data + {{ .FaceGroupDataOffset }},
  .objects = (void *)_{{ .Name }}_data + {{ .ObjectDataOffset }}
};