#ifndef {{ .Name }}_bpl_section
#define {{ .Name }}_bpl_section {{ if not .CpuOnly }}__data_chip{{ else }}__data{{ end }} 
#endif

static {{ .Name }}_bpl_section u_short _{{ .Name }}_bpl[] = {
  {{ range .BplData }}
  {{- . -}},
  {{ end -}}
};

#define {{ .Name }}_width {{ .Width }}
#define {{ .Name }}_height {{ .Height }}
#define {{ .Name }}_depth {{ .Depth }}
#define {{ .Name }}_bytesPerRow {{ .BytesPerRow }}
#define {{ .Name }}_bplSize {{ .BplSize }}
#define {{ .Name }}_size {{ .Size}}
{{ if not .OnlyData }}
{{ if not .Shared }}static {{ end }}const __data BitmapT {{ .Name }} = {
  .width = {{ .Width }},
  .height = {{ .Height }},
  .depth = {{ .Depth }},
  .bytesPerRow = {{ .BytesPerRow }},
  .bplSize = {{ .BplSize }},
  .flags = {{ .Flags }},
  .planes = {
    {{ range .BplPtrs }}
    {{- . -}},
    {{ end -}}
  }
};
{{ end }}

