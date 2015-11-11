uniform sampler2D keys;
void main()
{
  gl_FragColor = texture2D(keys, gl_TexCoord[0].st);
  gl_FragColor.a = .5;
}
