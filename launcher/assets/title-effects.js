/** Textured reconstruction of the soft, flowing Pokemon Box title energy. */
export function mountTitleEffects(container, {theme = 'fire', seed} = {}) {
  if (!container?.appendChild) throw new TypeError('Title effects need a container.');
  if (!['fire', 'leaf'].includes(theme)) throw new TypeError('Unknown title effect theme.');
  const doc = container.ownerDocument;
  const win = doc.defaultView;
  const controller = new AbortController();
  const images = new Set();
  const makeCanvas = className => {
    const node = doc.createElement('canvas');
    node.width = 1280;
    node.height = 720;
    node.className = className;
    node.dataset.theme = theme;
    node.setAttribute('aria-hidden', 'true');
    Object.assign(node.style, {position:'absolute', inset:'0', width:'100%', height:'100%',
      pointerEvents:'none', display:'block', imageRendering:'auto'});
    return node;
  };
  let canvas = makeCanvas(`title-effects title-effects-${theme}`);
  let renderer;
  try {
    renderer = makeWebGL(canvas);
  } catch {
    // A canvas that already acquired WebGL cannot subsequently acquire 2D.
    canvas = makeCanvas(`title-effects title-effects-${theme}`);
    renderer = makeStatic(canvas);
  }
  container.appendChild(canvas);
  canvas.dataset.renderer=renderer.webgl?'webgl':'2d-warp';
  canvas.dataset.frame='0';
  const backCanvas=makeCanvas(theme==='leaf'?'title-leaf-sprites title-leaf-back':'title-fire-sprites title-fire-back');
  const backInk = backCanvas?.getContext('2d');
  if (backCanvas && backInk) container.appendChild(backCanvas);
  const frontCanvas=makeCanvas(theme==='leaf'?'title-leaf-sprites title-leaf-front':'title-fire-sprites title-fire-front');
  const frontInk = frontCanvas?.getContext('2d');
  if(frontCanvas && frontInk) {
    frontCanvas.style.zIndex='3';
    (container.closest('#screen') || container).appendChild(frontCanvas);
  }
  canvas.dataset.leaves=theme==='leaf'?'44':'0';

  let randomState = 2166136261;
  const seedText = String(seed ?? Math.floor(Math.random() * 0x100000000));
  for (let index = 0; index < seedText.length; index++) {
    randomState = Math.imul(randomState ^ seedText.charCodeAt(index), 16777619) >>> 0;
  }
  const random = () => {
    randomState += 0x6d2b79f5;
    let value = randomState;
    value = Math.imul(value ^ value >>> 15, value | 1);
    value ^= value + Math.imul(value ^ value >>> 7, value | 61);
    return ((value ^ value >>> 14) >>> 0) / 4294967296;
  };
  const phase = random() * Math.PI * 2;
  const leaves = Array.from({length:44}, (_, index) => ({
    foreground:index>=24, offset:random()*1700, phase:random()*Math.PI*2,
    speed:70+random()*80, y:140+(index%11)*84+random()*42,
    size:[24,32,40,48,56,64][index>=24?2+index%4:index%4],
    alpha:index>=24?.70+random()*.24:.38+random()*.27,
    reverse:index%3===0, flip:index%2===0, spin:(random()-.5)*1.6
  }));
  const media = win.matchMedia('(prefers-reduced-motion: reduce)');
  let reducedMotion = media.matches;
  let disposed = false, loaded = false, contextLost = false;
  let frameId = 0, elapsed = 0, lastTime = null, lastDraw = null;
  let textureImage=null,leafImage=null,leafDefinition=null,fireImage=null,fireDefinition=null;
  let recoveryCanvas=null, recoveryRenderer=null, paintedFrames=0;

  function makeWebGL(node) {
    const gl = node.getContext('webgl', {alpha:true, antialias:false, depth:false,
      stencil:false, premultipliedAlpha:false, powerPreference:'low-power'});
    if (!gl) throw new Error('WebGL is unavailable.');
    const shaders = [];
    let program = null, buffer = null, texture = null;
    const release = () => {
      if (gl.isContextLost()) return;
      if (texture) gl.deleteTexture(texture);
      if (buffer) gl.deleteBuffer(buffer);
      if (program) gl.deleteProgram(program);
      shaders.forEach(shader => gl.deleteShader(shader));
    };
    try {
      const compile = (type, source) => {
        const shader = gl.createShader(type);
        if (!shader) throw new Error('Could not create the title shader.');
        shaders.push(shader);
        gl.shaderSource(shader, source);
        gl.compileShader(shader);
        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) throw new Error('Could not compile the title shader.');
        return shader;
      };
      const vertex = compile(gl.VERTEX_SHADER, `
        attribute vec2 position;
        varying vec2 uv;
        void main() { uv = position * .5 + .5; gl_Position = vec4(position, 0., 1.); }
      `);
      const fragment = compile(gl.FRAGMENT_SHADER, `
        precision mediump float;
        varying vec2 uv;
        uniform sampler2D energy;
        uniform vec2 crop;
        uniform float clock;
        uniform float phase;
        uniform float leaf;
        vec3 rgbToHsv(vec3 c) {
          vec4 K=vec4(0.,-1./3.,2./3.,-1.);
          vec4 p=mix(vec4(c.bg,K.wz),vec4(c.gb,K.xy),step(c.b,c.g));
          vec4 q=mix(vec4(p.xyw,c.r),vec4(c.r,p.yzx),step(p.x,c.r));
          float d=q.x-min(q.w,q.y), e=.00001;
          return vec3(abs(q.z+(q.w-q.y)/(6.*d+e)),d/(q.x+e),q.x);
        }
        vec3 hsvToRgb(vec3 c) {
          vec3 p=abs(fract(c.xxx+vec3(0.,2./3.,1./3.))*6.-3.);
          return c.z*mix(vec3(1.),clamp(p-1.,0.,1.),c.y);
        }
        void main() {
          // Two continuous texture fields travel independently. Clamped edges
          // retain the bright lower field without introducing a wrapping seam.
          float rise=uv.y*9.-clock*1.65;
          float longWave=sin(rise+sin(uv.x*6.+phase)*.85);
          float fineWave=sin(uv.y*17.-clock*2.30+uv.x*8.+phase);
          float strength=.6+.4*uv.y;
          vec2 flow=vec2((longWave*.066+fineWave*.019)*strength,
            sin(uv.x*8.+uv.y*5.-clock*1.35+phase)*.052);
          vec2 sampleUv=(uv+flow-.5)*crop+.5;
          vec4 color=texture2D(energy,clamp(sampleUv,vec2(.001),vec2(.999)));
          vec2 secondFlow=vec2(sin(uv.y*11.-clock*.91+phase)*.051,
            cos(uv.x*7.+uv.y*3.-clock*.79+phase)*.041);
          vec2 secondUv=(uv+secondFlow-.5)*crop+.5;
          vec4 secondary=texture2D(energy,clamp(secondUv,vec2(.001),vec2(.999)));
          float layerMix=.18+.055*sin(uv.y*8.-clock*1.70+uv.x*3.);
          color=mix(color,secondary,layerMix);
          if (leaf > .5) {
            vec3 hsv=rgbToHsv(color.rgb);
            float pink=smoothstep(.70,.91,hsv.x);
            if(hsv.x<.16) pink=1.;
            float shade=mix(1.,mix(.62,1.,pink),smoothstep(.15,.65,hsv.y));
            vec3 green=hsvToRgb(vec3(mix(.46,.28,pink),min(.9,hsv.y*.90),hsv.z*shade));
            color.rgb=mix(color.rgb,green,smoothstep(.025,.14,hsv.y));
          } else {
            vec3 hsv=rgbToHsv(color.rgb);
            float pink=smoothstep(.70,.91,hsv.x);
            if(hsv.x<.16)pink=1.;
            float shade=mix(1.,mix(.60,1.,pink),smoothstep(.15,.65,hsv.y));
            vec3 warm=hsvToRgb(vec3(fract(mix(.99,1.085,pink)),min(.98,hsv.y*1.02),hsv.z*shade));
            color.rgb=mix(color.rgb,warm,smoothstep(.025,.14,hsv.y));
          }
          float vivid=smoothstep(.08,.5,rgbToHsv(color.rgb).y);
          float beam=pow(.5+.5*sin(uv.x*5.+uv.y*3.-clock*.72+phase),3.);
          float glow=pow(.5+.5*sin(uv.y*5.-clock*1.45+uv.x*3.),2.);
          float illumination=.86-.22*smoothstep(.24,.98,uv.y)+.25*beam+.13*glow;
          color.rgb=mix(color.rgb,color.rgb*illumination,vivid);
          color.rgb=mix(color.rgb,vec3(1.),.08*glow*(1.-.7*uv.y)*vivid);
          gl_FragColor=color;
        }
      `);
      program = gl.createProgram();
      if (!program) throw new Error('Could not create the title program.');
      gl.attachShader(program, vertex);
      gl.attachShader(program, fragment);
      gl.linkProgram(program);
      if (!gl.getProgramParameter(program, gl.LINK_STATUS)) throw new Error('Could not link the title shader.');
      buffer = gl.createBuffer();
      texture = gl.createTexture();
      if (!buffer || !texture) throw new Error('Could not allocate the title texture.');
      gl.useProgram(program);
      gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1,-1,1,-1,-1,1,1,1]), gl.STATIC_DRAW);
      const position = gl.getAttribLocation(program, 'position');
      gl.enableVertexAttribArray(position);
      gl.vertexAttribPointer(position, 2, gl.FLOAT, false, 0, 0);
      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, texture);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
      gl.uniform1i(gl.getUniformLocation(program, 'energy'), 0);
      gl.uniform1f(gl.getUniformLocation(program, 'leaf'), theme === 'leaf' ? 1 : 0);
      const clockUniform = gl.getUniformLocation(program, 'clock');
      const phaseUniform = gl.getUniformLocation(program, 'phase');
      const cropUniform = gl.getUniformLocation(program, 'crop');
      gl.viewport(0, 0, node.width, node.height);
      return {
        webgl:true,
        upload(image) {
          gl.bindTexture(gl.TEXTURE_2D, texture);
          gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
          gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
          const aspect = image.naturalWidth / image.naturalHeight;
          const target = 1280 / 720;
          gl.uniform2f(cropUniform, Math.min(1,target/aspect), Math.min(1,aspect/target));
        },
        draw(time, offset) {
          gl.uniform1f(clockUniform, time);
          gl.uniform1f(phaseUniform, offset);
          gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        },
        dispose:release
      };
    } catch (error) {
      release();
      throw error;
    }
  }

  function makeStatic(node) {
    const output=node.getContext('2d');
    if(!output)throw new Error('Title image rendering is unavailable.');
    // Keep the fallback affordable on machines without a GPU. The source is
    // intentionally soft, so a linear upscale retains its texture character.
    const pad=72,width=640,height=360;
    const warped=doc.createElement('canvas');
    warped.width=width;warped.height=height;
    const context=warped.getContext('2d');
    if(!context)throw new Error('Title image rendering is unavailable.');
    let plane=null;
    return {
      webgl:false,
      upload(source) {
        const image=tintPaletteImage(source);
        plane=doc.createElement('canvas');
        plane.width=width+pad*2;
        plane.height=height+pad*2;
        const paint=plane.getContext('2d');
        const imageWidth=image.naturalWidth||image.width;
        const imageHeight=image.naturalHeight||image.height;
        const scale=Math.max(width/imageWidth,height/imageHeight);
        paint.imageSmoothingEnabled=true;
        paint.imageSmoothingQuality='high';
        paint.drawImage(image,pad+(width-imageWidth*scale)/2,pad+(height-imageHeight*scale)/2,imageWidth*scale,imageHeight*scale);
        // Extend edge texels instead of wrapping or exposing empty slice edges.
        paint.drawImage(plane,pad,pad,1,height,0,pad,pad,height);
        paint.drawImage(plane,pad+width-1,pad,1,height,pad+width,pad,pad,height);
        paint.drawImage(plane,0,pad,plane.width,1,0,0,plane.width,pad);
        paint.drawImage(plane,0,pad+height-1,plane.width,1,0,pad+height,plane.width,pad);
      },
      draw(time,offset) {
        if(!plane)return;
        context.imageSmoothingEnabled=true;
        context.imageSmoothingQuality='high';
        for(let pass=0;pass<2;pass++) {
          context.globalAlpha=pass?.19:1;
          for(let y=0;y<height;y+=6) {
            const uv=1-(y+3)/height;
            const dx=pass?Math.sin(uv*11-time*.91+offset)*.051
              :(Math.sin(uv*9-time*1.65+offset)*.066+Math.sin(uv*17-time*2.30+offset)*.019)*(.6+.4*uv);
            const dy=pass?Math.cos(uv*3-time*.79+offset)*.041
              :Math.sin(uv*5-time*1.35+offset)*.052;
            context.drawImage(plane,pad+dx*width,pad+y-dy*height,width,6,0,y,width,6);
          }
        }
        context.globalAlpha=1;
        const shade=context.createLinearGradient(0,0,0,height);
        shade.addColorStop(0,theme==='leaf'?'rgba(0,26,21,.36)':'rgba(35,2,0,.36)');
        shade.addColorStop(.72,'rgba(0,0,0,.02)');
        shade.addColorStop(1,'rgba(0,0,0,0)');
        context.fillStyle=shade;
        context.fillRect(0,0,width,height);
        const x=width*.5+Math.sin(time*.72+offset)*width*.4;
        const beam=context.createLinearGradient(x-470,0,x+470,height);
        beam.addColorStop(0,'rgba(255,255,255,0)');
        beam.addColorStop(.5,'rgba(255,255,255,.15)');
        beam.addColorStop(1,'rgba(255,255,255,0)');
        context.fillStyle=beam;
        context.fillRect(0,0,width,height);
        output.imageSmoothingEnabled=true;
        output.imageSmoothingQuality='high';
        output.drawImage(warped,0,0,node.width,node.height);
      },
      dispose(){plane=null;}
    };
  }

  function tintPaletteImage(source) {
    const tinted = doc.createElement('canvas');
    tinted.width = source.naturalWidth;
    tinted.height = source.naturalHeight;
    const context = tinted.getContext('2d', {willReadFrequently:true});
    if (!context) return source;
    context.drawImage(source,0,0);
    const pixels = context.getImageData(0,0,tinted.width,tinted.height);
    const data = pixels.data;
    const smooth = (lo,hi,value) => { const x=Math.max(0,Math.min(1,(value-lo)/(hi-lo)));return x*x*(3-2*x); };
    for (let index=0;index<data.length;index+=4) {
      const r=data[index]/255,g=data[index+1]/255,b=data[index+2]/255;
      const value=Math.max(r,g,b),delta=value-Math.min(r,g,b);
      const saturation=value ? delta/value : 0;
      if(saturation<=.025)continue;
      let hue=delta ? (value===r?(g-b)/delta+(g<b?6:0):value===g?(b-r)/delta+2:(r-g)/delta+4)/6 : 0;
      const pink=hue<.16?1:smooth(.70,.91,hue);
      hue=theme==='leaf'?.46+(.28-.46)*pink:(.99+.095*pink)%1;
      const dark=theme==='leaf'?.62:.60;
      const shade=1+(dark+(1-dark)*pink-1)*smooth(.15,.65,saturation);
      const tintedValue=value*shade;
      const sat=theme==='leaf'?Math.min(.9,saturation*.9):Math.min(.98,saturation*1.02);
      const chroma=tintedValue*sat;
      const h=hue*6,x=chroma*(1-Math.abs(h%2-1)),m=tintedValue-chroma;
      const rgb=h<1?[chroma,x,0]:h<2?[x,chroma,0]:h<3?[0,chroma,x]:h<4?[0,x,chroma]:h<5?[x,0,chroma]:[chroma,0,x];
      const mix=smooth(.025,.14,saturation);
      for(let channel=0;channel<3;channel++)data[index+channel]=Math.round(data[index+channel]*(1-mix)+(rgb[channel]+m)*255*mix);
    }
    context.putImageData(pixels,0,0);
    return tinted;
  }

  async function loadImage(url) {
    const image = new win.Image();
    images.add(image);
    image.decoding = 'async';
    image.src = url.href;
    try { await image.decode(); return image; }
    finally { images.delete(image); }
  }

  function drawLeaves(time) {
    if(!backInk || !leafImage || !leafDefinition)return;
    const sprite=leafDefinition;
    backInk.clearRect(0,0,1280,720);
    frontInk?.clearRect(0,0,1280,720);
    const smooth=(lo,hi,value)=>{const x=Math.max(0,Math.min(1,(value-lo)/(hi-lo)));return x*x*(3-2*x);};
    leaves.forEach((leaf,index)=>{
      if(reducedMotion && index%4)return;
      const ink=leaf.foreground?frontInk:backInk;
      if(!ink)return;
      const distance=(time*leaf.speed+leaf.offset)%1700;
      let x=-210+distance;
      if(leaf.reverse)x=1280-x;
      x+=Math.sin(time*.95+leaf.phase)*46;
      const y=leaf.y-distance*.31+Math.sin(time*.83+leaf.phase)*48;
      const logo=smooth(275,430,x)*(1-smooth(850,1005,x))*(1-smooth(255,355,y));
      const alpha=leaf.alpha*(1-.94*logo);
      const size=Math.round(leaf.size*(.94+.06*Math.sin(time*1.3+leaf.phase)));
      const frame=sprite.sequence[Math.floor((time*1000+leaf.phase*700)/sprite.frameMs)%sprite.sequence.length];
      ink.save();
      ink.imageSmoothingEnabled=false;
      ink.translate(Math.round(x),Math.round(y));
      ink.rotate(time*leaf.spin+Math.sin(time*1.1+leaf.phase)*.65);
      if(leaf.flip)ink.scale(-1,1);
      ink.globalAlpha=alpha;
      if(leaf.foreground){ink.shadowColor='rgba(0,28,12,.24)';ink.shadowBlur=2;ink.shadowOffsetX=2;ink.shadowOffsetY=3;}
      ink.drawImage(leafImage,(frame%sprite.columns)*sprite.frameWidth,
        Math.floor(frame/sprite.columns)*sprite.frameHeight,sprite.frameWidth,sprite.frameHeight,
        -size/2,-size/2,size,size);
      ink.restore();
    });
    if(frontCanvas)frontCanvas.dataset.frame=String(paintedFrames);
  }

  function drawFireRows(time) {
    if(!backInk || !frontInk || !fireImage || !fireDefinition)return;
    const sprite=fireDefinition;
    const interval=sprite.originalSpawner.intervalMs/1000;
    const lifetime=sprite.durationMs/1000;
    backInk.clearRect(0,0,1280,720);
    frontInk.clearRect(0,0,1280,720);
    let submitted=0;
    const noise=(row,column,layer)=>{
      const value=Math.sin(row*17.13+column*71.71+layer*39.4+phase)*43758.5453;
      return value-Math.floor(value);
    };
    for(let layer=0;layer<2;layer++) {
      const ink=layer?frontInk:backInk;
      const clock=time+.61+(layer?.17:0);
      const newest=Math.floor(clock/interval);
      const columns=layer?24:32;
      for(let row=0;row<4;row++) {
        const birth=newest-row;
        const age=clock-birth*interval;
        if(age<0 || age>=lifetime)continue;
        let remaining=age*1000,frameIndex=0;
        while(frameIndex<sprite.frameDurationsMs.length-1 && remaining>=sprite.frameDurationsMs[frameIndex]) {
          remaining-=sprite.frameDurationsMs[frameIndex++];
        }
        const frame=sprite.sequence[frameIndex];
        // The source's last frames turn olive/teal. Fade their original palette
        // rather than recoloring those pixels or looping an individual flare.
        let fade=Math.max(0,Math.min(1,(.78-age)/.30));
        if(frameIndex>=7)fade*=.12;
        if(fade<.025)continue;
        const rowOffset=((birth%4+4)%4)*3+(row%2?640/columns:0);
        for(let column=0;column<columns;column++) {
          const variation=noise(birth,column,layer);
          const size=layer?64:(column%4===0?48:64);
          const x=-18+column*(1316/(columns-1))+rowOffset+(variation-.5)*14
            +age*(variation-.5)*29;
          const speed=layer?148+variation*30:210+variation*48;
          const y=(layer?760:650)+(variation-.5)*14-age*speed;
          const alpha=fade*(layer?.91:.86);
          ink.save();
          ink.imageSmoothingEnabled=false;
          ink.globalAlpha=alpha;
          ink.shadowColor=layer?'rgba(138,34,0,.55)':'rgba(116,29,0,.38)';
          ink.shadowBlur=layer?4:3;
          ink.shadowOffsetY=2;
          ink.drawImage(fireImage,(frame%sprite.columns)*sprite.frameWidth,
            Math.floor(frame/sprite.columns)*sprite.frameHeight,sprite.frameWidth,sprite.frameHeight,
            Math.round(x-size/2),Math.round(y-size/2),size,size);
          ink.restore();
          submitted++;
        }
      }
    }
    canvas.dataset.fireSprites=String(submitted);
    frontCanvas.dataset.frame=String(paintedFrames);
  }

  function draw(time) {
    if(!loaded || disposed)return;
    const active=contextLost?recoveryRenderer:renderer;
    if(!active)return;
    active.draw(time,phase);
    canvas.dataset.frame=String(++paintedFrames);
    if(theme==='leaf')drawLeaves(time);else drawFireRows(time);
  }

  function stop() {
    if (frameId) win.cancelAnimationFrame(frameId);
    frameId=0;
    lastTime=null;
    lastDraw=null;
  }

  function frame(now) {
    frameId=0;
    if (disposed || doc.hidden || reducedMotion || !loaded || (contextLost && !recoveryRenderer)) return;
    if(lastTime!==null)elapsed+=Math.min(.06,Math.max(0,(now-lastTime)/1000));
    lastTime=now;
    if(lastDraw===null || now-lastDraw>=1000/30-.5) {
      draw(elapsed);
      lastDraw=now;
    }
    frameId=win.requestAnimationFrame(frame);
  }

  function resume() {
    stop();
    if(disposed || doc.hidden || !loaded || (contextLost && !recoveryRenderer))return;
    draw(reducedMotion?0:elapsed);
    if(!reducedMotion)frameId=win.requestAnimationFrame(frame);
  }

  function motionChanged() { reducedMotion=media.matches;resume(); }
  function beginRecovery() {
    if(!textureImage || recoveryRenderer || disposed)return;
    recoveryCanvas=makeCanvas('title-effects title-effects-recovery');
    recoveryCanvas.dataset.renderer='2d-warp';
    recoveryRenderer=makeStatic(recoveryCanvas);
    recoveryRenderer.upload(textureImage);
    container.insertBefore(recoveryCanvas,canvas);
    canvas.style.opacity='0';
    canvas.dataset.renderer='2d-recovery';
  }
  function clearRecovery() {
    recoveryRenderer?.dispose();
    recoveryCanvas?.remove();
    recoveryRenderer=null;
    recoveryCanvas=null;
    canvas.style.opacity='1';
  }
  function lost(event) {
    if(disposed)return;
    event.preventDefault();
    contextLost=true;
    stop();
    beginRecovery();
    resume();
  }
  function restored() {
    if(disposed)return;
    stop();
    try {
      // The old GPU handles were released by context loss.
      renderer=makeWebGL(canvas);
      if(textureImage)renderer.upload(textureImage);
      contextLost=false;
      canvas.dataset.renderer='webgl';
      clearRecovery();
    } catch { /* Continue the animated 2D fallback if restoration fails. */ }
    resume();
  }
  canvas.addEventListener('webglcontextlost',lost);
  canvas.addEventListener('webglcontextrestored',restored);
  doc.addEventListener('visibilitychange',resume);
  if(media.addEventListener)media.addEventListener('change',motionChanged);
  else media.addListener(motionChanged);

  const ready=(async()=>{
    try {
      const textureUrl=new URL('./backgrounds/title-energy.png',import.meta.url);
      const texturePromise=loadImage(textureUrl);
      const leafPromise=theme==='leaf'?(async()=>{
        const metadataUrl=new URL('./title-sprites/metadata.json',import.meta.url);
        const response=await win.fetch(metadataUrl,{signal:controller.signal});
        if(!response.ok)throw new Error('Original leaf sprite metadata could not be loaded.');
        const metadata=await response.json(),sprite=metadata.leaf;
        if(!sprite || !Array.isArray(sprite.sequence) || !sprite.sequence.length
          || !sprite.sequence.every(index=>Number.isInteger(index)&&index>=0&&index<sprite.frames)
          || !(sprite.frameWidth>0) || !(sprite.frameHeight>0) || !(sprite.columns>0) || !(sprite.frameMs>0)) {
          throw new Error('Original leaf sprite metadata is invalid.');
        }
        const image=await loadImage(new URL(sprite.url,metadataUrl));
        if(image.naturalWidth<sprite.columns*sprite.frameWidth
          || image.naturalHeight<Math.ceil(sprite.frames/sprite.columns)*sprite.frameHeight) {
          throw new Error('Original leaf sprite sheet has unexpected dimensions.');
        }
        return {image,sprite};
      })():Promise.resolve(null);
      const firePromise=theme==='fire'?(async()=>{
        const metadataUrl=new URL('./title-sprites/title-fire.json',import.meta.url);
        const response=await win.fetch(metadataUrl,{signal:controller.signal});
        if(!response.ok)throw new Error('Original title fire metadata could not be loaded.');
        const sprite=await response.json();
        if(sprite.loop!==false || !Array.isArray(sprite.sequence) || !sprite.sequence.length
          || !Array.isArray(sprite.frameDurationsMs) || sprite.frameDurationsMs.length!==sprite.sequence.length
          || !sprite.frameDurationsMs.every(value=>value>0) || !(sprite.durationMs>0)
          || !(sprite.originalSpawner?.intervalMs>0)
          || !sprite.sequence.every(index=>Number.isInteger(index)&&index>=0&&index<sprite.frames)) {
          throw new Error('Original title fire metadata is invalid.');
        }
        const image=await loadImage(new URL(sprite.url,metadataUrl));
        if(image.naturalWidth<sprite.columns*sprite.frameWidth
          || image.naturalHeight<Math.ceil(sprite.frames/sprite.columns)*sprite.frameHeight) {
          throw new Error('Original title fire sprite sheet has unexpected dimensions.');
        }
        return {image,sprite};
      })():Promise.resolve(null);
      const [image,leaf,fire]=await Promise.all([texturePromise,leafPromise,firePromise]);
      if(disposed)return false;
      textureImage=image;
      if(leaf){leafImage=leaf.image;leafDefinition=leaf.sprite;}
      if(fire){fireImage=fire.image;fireDefinition=fire.sprite;}
      renderer.upload(textureImage);
      loaded=true;
      if(contextLost)beginRecovery();
      resume();
      return true;
    } catch(error) {
      if(disposed || error.name==='AbortError')return false;
      throw new Error(`Title energy image could not be loaded: ${error.message}`);
    }
  })();

  return {
    canvas,
    ready,
    dispose() {
      if(disposed)return;
      disposed=true;
      stop();
      controller.abort();
      images.forEach(image=>{image.src='';});
      images.clear();
      canvas.removeEventListener('webglcontextlost',lost);
      canvas.removeEventListener('webglcontextrestored',restored);
      doc.removeEventListener('visibilitychange',resume);
      if(media.removeEventListener)media.removeEventListener('change',motionChanged);
      else media.removeListener(motionChanged);
      renderer.dispose();
      clearRecovery();
      canvas.remove();
      backCanvas?.remove();
      frontCanvas?.remove();
      textureImage=null;
      leafImage=null;
      fireImage=null;
    }
  };
}
