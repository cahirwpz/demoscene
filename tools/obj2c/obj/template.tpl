static short _{{ .Name }}_vertex[{{ .VertexCount }} * 4] = {
  /* x, y, z, pad */
  {{- range .Vertices }}
  {{ range . }}{{ . }}, {{ end -}} 0,
{{- end }}
};

static short _{{ .Name }}_face_vertex[{{ .FaceDataCount }}] = {
  /* #vertices, vertices... */
  {{- range .Faces }}
  {{range . }}{{ . }}, {{ end -}}
{{- end}}
  0 
};

{{- if .Edges }}

static short _{{ .Name }}_edges[{{ .EdgeCount }} * 2] = {
  {{- range .Edges }}
  {{ range . }}{{ . }}, {{ end -}}
{{- end }}
};

static short _{{ .Name }}_face_edge[{{ .FaceDataCount }}] = {
  /* #edge, edges... */
  {{- range .FaceEdges }}
  {{range . }}{{ . }}, {{ end -}}
{{- end}}
  0 
};
{{- end }}

{{- if .FaceNormals }}

static short _{{ .Name }}_face_normals[{{ .FaceCount }} * 4] = {
  /* x, y, z, pad */
  {{- range .FaceNormals }}
  {{ range . }}{{ . }}, {{ end -}} 0,
{{- end }}
};
{{- end }}

Mesh3D {{ .Name }} = {
  .vertices = {{ .VertexCount }},
  .faces = {{ .FaceCount }},
  .edges = {{ .EdgeCount }},
  .vertex = (Point3D *)&_{{ .Name }}_vertex,
{{- if .FaceNormals }}
  .faceNormal = (Point3D *)&_{{ .Name }}_face_normals,
{{- else }}
  .faceNormal = NULL,{{ end }}
{{- if .Edges }}
  .edge = (EdgeT *)&_{{ .Name }}_edges,
{{- else }}
  .edge = NULL,{{ end }}
  .faceVertex = _{{ .Name }}_face_vertex,
{{- if .Edges }}
  .faceEdge = _{{ .Name }}_face_edge,
{{- else }}
  .faceEdge = NULL,{{ end }}
};
