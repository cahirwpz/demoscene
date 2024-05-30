static short _{{ .Name }}_pnts[{{ .VertexCount }} * 4] = {
  /* x, y, z, pad */
  {{- range .Vertices }}
  {{ range . }}{{ . }}, {{ end -}} 0,
{{- end }}
};

static short _{{ .Name }}_face_data[{{ .FaceDataCount }}] = {
  /* #vertices, vertices... */
  {{- range .Faces }}
  {{range . }}{{ . }}, {{ end -}}
{{- end}}
  0 
};

static short *_{{ .Name }}_face[{{ .FaceCount }} + 1] = {
  {{- range .FaceIndices }}
  &_{{ $.Name }}_face_data[{{ . }}],
{{- end }}
  NULL
};

Mesh3D {{ .Name }} = {
  .vertices = {{ .VertexCount }},
  .faces = {{ .FaceCount }},
  .edges = 0,
  .vertex = (Point3D *)&_{{ .Name }}_pnts,
  .faceNormal = NULL,
  .vertexNormal = NULL,
  .edge = NULL,
  .face = _{{ .Name }}_face,
  .faceEdge = NULL,
  .vertexFace = NULL,
};
