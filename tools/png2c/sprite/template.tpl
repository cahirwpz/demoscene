#define {{.Name}}_height {{.Height}}

{{ range .Sprites -}}
static __data_chip SprDataT {{.Name}} = {
  .pos = SPRPOS(0, 0),
  .ctl = SPRCTL(0, 0, {{.Attached}}, {{.Height}}),
  .data = {
    {{ range .Data -}}
    {{ . }}
    {{ end -}}
  }
};
{{ end -}}

{{- if .Array }}
#define {{.Name}}_sprites {{.Count}}

static __data SprDataT *{{.Name}}[{{.Count}}] = {
  {{ range .Sprites -}}
  &{{.Name}},
  {{ end -}}
};
{{ end -}}
