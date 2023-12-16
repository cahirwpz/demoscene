#define {{ .Name }}_colors_count {{ .Count }}

{{if not .Shared }}static {{ end }}__data u_short {{ .Name }}_colors[{{ .Count }}] = {
  {{ range .ColorsData }}
  {{- . -}},
  {{ end -}}
};

