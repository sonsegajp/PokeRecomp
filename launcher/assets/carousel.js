import * as THREE from './vendor/three/three.module.js';

// Actual textured box geometry. Cover scans stay separate from every mesh and UI label.
class GameCarousel {
  constructor(container,games,{index=2,onSelect=()=>{}}={}) {
    this.container=container;this.games=games;this.index=index;this.onSelect=onSelect;
    this.scene=new THREE.Scene();this.boxes=[];this.disposed=false;
    this.camera=new THREE.PerspectiveCamera(34,1,.1,60);this.camera.position.set(0,.85,7.4);this.camera.lookAt(0,0,0);
    this.renderer=new THREE.WebGLRenderer({alpha:true,antialias:true,powerPreference:'low-power'});
    this.renderer.setPixelRatio(Math.min(devicePixelRatio,2));this.renderer.setClearColor(0x000000,0);
    this.renderer.outputColorSpace=THREE.SRGBColorSpace;this.renderer.shadowMap.enabled=true;this.renderer.shadowMap.type=THREE.PCFSoftShadowMap;
    this.renderer.domElement.setAttribute('aria-hidden','true');container.appendChild(this.renderer.domElement);
    this.scene.add(new THREE.AmbientLight(0xffffff,2.1));
    const key=new THREE.DirectionalLight(0xffead5,3.3);key.position.set(-3,7,6);key.castShadow=true;
    key.shadow.mapSize.set(1024,1024);key.shadow.camera.left=-7;key.shadow.camera.right=7;key.shadow.camera.top=4;key.shadow.camera.bottom=-4;key.shadow.normalBias=.03;this.scene.add(key);
    const fill=new THREE.DirectionalLight(0x9cbdff,1.1);fill.position.set(5,2,-1);this.scene.add(fill);
    const ground=new THREE.Mesh(new THREE.PlaneGeometry(30,15),new THREE.ShadowMaterial({color:0x07101b,opacity:.32}));ground.rotation.x=-Math.PI/2;ground.position.y=-1.38;ground.receiveShadow=true;this.scene.add(ground);
    const loader=new THREE.TextureLoader();
    games.forEach((game,i)=>{
      // games that aren't available yet sit on the shelf greyed out
      const cover=game.disabled?this.greyCover(`assets/covers/${game.id}.png`,ratio=>this.fitBox(model,edge,ratio)):loader.load(`assets/covers/${game.id}.png`,texture=>{
        if(this.disposed){texture.dispose();return;}
        const ratio=texture.image.width/texture.image.height;
        // Retail GBA cartons are nearly square; retain the scan's true aspect ratio.
        const height=2.5,width=height*ratio;
        model.geometry.dispose();model.geometry=new THREE.BoxGeometry(width,height,.34);
        edge.geometry.dispose();edge.geometry=new THREE.EdgesGeometry(model.geometry);
        model.userData.width=width;
      });cover.colorSpace=THREE.SRGBColorSpace;cover.anisotropy=this.renderer.capabilities.getMaxAnisotropy();
      const side=this.spineTexture(game);
      const front=new THREE.MeshStandardMaterial({map:cover,roughness:.77,metalness:0});
      const spine=new THREE.MeshStandardMaterial({map:side,roughness:.87});
      const paper=new THREE.MeshStandardMaterial({color:game.disabled?0x6b6f75:game.color,roughness:.92});
      const back=new THREE.MeshStandardMaterial({color:game.disabled?0x6b6f75:game.color,roughness:.92});
      const model=new THREE.Mesh(new THREE.BoxGeometry(2.25,2.5,.34),[spine,spine,paper,paper,front,back]);
      model.castShadow=true;model.receiveShadow=true;model.userData={index:i,width:2.25};
      const edge=new THREE.LineSegments(new THREE.EdgesGeometry(model.geometry),new THREE.LineBasicMaterial({color:0xffc580,transparent:true,opacity:0}));model.add(edge);
      model.userData.edge=edge;model.position.set((i-index)*2.05,0,-Math.abs(i-index)*.9);this.scene.add(model);this.boxes.push(model);
    });
    this.raycaster=new THREE.Raycaster();this.pointer=new THREE.Vector2();
    this.click=event=>{const bounds=this.renderer.domElement.getBoundingClientRect();this.pointer.set((event.clientX-bounds.left)/bounds.width*2-1,-(event.clientY-bounds.top)/bounds.height*2+1);this.raycaster.setFromCamera(this.pointer,this.camera);const hit=this.raycaster.intersectObjects(this.boxes,false)[0];if(hit)this.onSelect(hit.object.userData.index);};
    this.renderer.domElement.addEventListener('click',this.click);
    this.observer=new ResizeObserver(()=>this.resize());this.observer.observe(container);this.resize();this.animate();
  }
  // A game that isn't available yet: its cover drawn greyed out (a fixed-size
  // canvas: the GPU texture is allocated once, before the cover has loaded).
  greyCover(url,onload){
    const canvas=document.createElement('canvas');canvas.width=canvas.height=1024;
    const pre=canvas.getContext('2d');pre.fillStyle='#6b6f75';pre.fillRect(0,0,canvas.width,canvas.height);   // grey until the cover loads
    const texture=new THREE.CanvasTexture(canvas),img=new Image();
    texture.colorSpace=THREE.SRGBColorSpace;
    img.onload=()=>{
      if(this.disposed)return;
      const ctx=canvas.getContext('2d');ctx.filter='grayscale(1) brightness(1.05) contrast(.85)';
      ctx.drawImage(img,0,0,canvas.width,canvas.height);
      texture.needsUpdate=true;onload(img.naturalWidth/img.naturalHeight);
    };
    img.src=url;return texture;
  }
  fitBox(model,edge,ratio){
    if(this.disposed)return;
    // Retail GBA cartons are nearly square; retain the scan's true aspect ratio.
    const height=2.5,width=height*ratio;
    model.geometry.dispose();model.geometry=new THREE.BoxGeometry(width,height,.34);
    edge.geometry.dispose();edge.geometry=new THREE.EdgesGeometry(model.geometry);
    model.userData.width=width;
  }
  spineTexture(game){
    const canvas=document.createElement('canvas');canvas.width=128;canvas.height=1024;
    const ctx=canvas.getContext('2d');if(game.disabled)ctx.filter='grayscale(1)';ctx.fillStyle=game.color;ctx.fillRect(0,0,128,1024);ctx.fillStyle='#d5d7d6';ctx.fillRect(0,0,128,235);
    ctx.save();ctx.translate(64,118);ctx.rotate(-Math.PI/2);ctx.fillStyle='#22262e';ctx.textAlign='center';ctx.font='bold 31px Arial';ctx.fillText('GAME BOY',0,0);ctx.font='bold 23px Arial';ctx.fillText('ADVANCE',0,29);ctx.restore();
    ctx.save();ctx.translate(64,610);ctx.rotate(-Math.PI/2);ctx.textAlign='center';ctx.font='bold 45px Arial';ctx.lineWidth=9;ctx.strokeStyle='#203b70';ctx.strokeText('Pokémon '+game.name,0,0);ctx.fillStyle='#ffe348';ctx.fillText('Pokémon '+game.name,0,0);ctx.restore();
    const texture=new THREE.CanvasTexture(canvas);texture.colorSpace=THREE.SRGBColorSpace;return texture;
  }
  resize(){const width=this.container.clientWidth,height=this.container.clientHeight;if(!width||!height)return;this.renderer.setSize(width,height,false);this.camera.aspect=width/height;this.camera.updateProjectionMatrix();}
  setIndex(index){this.index=index;}
  animate(){
    if(this.disposed)return;
    this.frame=requestAnimationFrame(()=>this.animate());
    this.boxes.forEach((box,i)=>{
      let delta=i-this.index;
      if(delta>this.games.length/2)delta-=this.games.length;
      if(delta< -this.games.length/2)delta+=this.games.length;
      const active=delta===0;
      const x=delta*2.12,y=active?.03:-.12,z=active?.85:-Math.abs(delta)*.9;
      box.position.x=THREE.MathUtils.lerp(box.position.x,x,.11);box.position.y=THREE.MathUtils.lerp(box.position.y,y,.11);box.position.z=THREE.MathUtils.lerp(box.position.z,z,.11);
      box.rotation.y=THREE.MathUtils.lerp(box.rotation.y,active?-.16:-Math.sign(delta)*.35,.1);
      const scale=active?1.05:.86;box.scale.lerp(new THREE.Vector3(scale,scale,scale),.1);
      box.userData.edge.material.opacity=THREE.MathUtils.lerp(box.userData.edge.material.opacity,active?.72:0,.1);
    });
    this.renderer.render(this.scene,this.camera);
  }
  dispose(){this.disposed=true;cancelAnimationFrame(this.frame);this.observer.disconnect();this.renderer.domElement.removeEventListener('click',this.click);this.scene.traverse(obj=>{obj.geometry?.dispose();const materials=Array.isArray(obj.material)?obj.material:[obj.material];for(const material of materials){material?.map?.dispose();material?.dispose();}});this.renderer.dispose();this.renderer.domElement.remove();}
}
window.GameCarousel=GameCarousel;
window.dispatchEvent(new Event('carousel-ready'));
