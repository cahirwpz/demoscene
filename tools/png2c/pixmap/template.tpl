static {{ if .Displayable }}__data_chip {{ end }}{{ .PixType }} {{ .Name }}_pixels[{{ .Size }}] = {
  {{ range .PixData }}
    {{- . -}},
  {{ end -}}
};

#define {{ .Name }}_width {{ .Width }}
#define {{ .Name }}_height {{ .Height }}
{{ if not .OnlyData }}
static const __data PixmapT {{ .Name }} = {
  .type = {{ .Type }},
  .width = {{ .Width }},
  .height = {{ .Height }},
  .pixels = {{ .Name }}_pixels
};
{{ end -}}

