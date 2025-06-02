#define {{.Name}}_height {{.Height}}

{{ range .Sprites -}}
static __data_chip SprDataT {{.Name}}_sprdat[] = {
  { SPRPOS(0, 0), SPRCTL(0, 0, {{.Attached}}, {{.Height}}) },
  {{ range .Data -}}
  {{ . }}
  {{ end -}}
  SPREND()
};
{{ end }}

{{ range .Sprites -}}
#define {{.Name}} ((SpriteT *){{.Name}}_sprdat)
{{ end -}}

{{- if .Array }}
#define {{.Name}}_sprites {{.Count}}

static __data SpriteT *{{.Name}}[{{.Count}}] = {
  {{ range .Sprites -}}
  {{.Name}},
  {{ end -}}
};
{{ end -}}
