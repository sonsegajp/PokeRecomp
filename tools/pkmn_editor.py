#!/usr/bin/env python3
"""PkmnEditor — Game asset editor for PokeRecomp."""
import struct, os, sys, json, tkinter as tk
from tkinter import ttk, filedialog, messagebox, colorchooser
from PIL import Image, ImageTk, ImageDraw

ASSET_DIR = "extracted_assets/frlg"

def read_4bpp(data, pal, wt):
    tiles=[]
    for t in range(0,len(data),32):
        tile=[]
        for r in range(8):
            for c in range(0,8,2):
                i=t+r*4+c//2; b=data[i] if i<len(data) else 0
                tile.append(b&0xF); tile.append((b>>4)&0xF)
        tiles.append(tile)
    if not tiles: return Image.new('RGBA',(8,8))
    ht=max(1,len(tiles)//wt)
    img=Image.new('RGBA',(wt*8,ht*8),(0,0,0,0)); px=img.load()
    for ty in range(ht):
        for tx in range(wt):
            ti=ty*wt+tx
            if ti>=len(tiles): continue
            for py in range(8):
                for ppx in range(8):
                    ci=tiles[ti][py*8+ppx]; x,y=tx*8+ppx,ty*8+py
                    if ci==0: px[x,y]=(0,0,0,0)
                    else: px[x,y]=(*pal[ci%len(pal)],255)
    return img

def read_gbapal(path):
    d=open(path,'rb').read(); c=[]
    for i in range(0,min(len(d),32),2):
        v=struct.unpack_from('<H',d,i)[0]
        c.append(((v&0x1F)<<3,((v>>5)&0x1F)<<3,((v>>10)&0x1F)<<3))
    while len(c)<16: c.append((0,0,0))
    return c

def write_gbapal(path, colors):
    d=bytearray()
    for r,g,b in colors: d+=struct.pack('<H',(r>>3)|((g>>3)<<5)|((b>>3)<<10))
    open(path,'wb').write(d)

GRAY=[(i*17,)*3 for i in range(16)]

class PkmnEditor:
    def __init__(self, root, ad):
        self.root=root; self.ad=ad
        root.title(f"PkmnEditor — {os.path.basename(os.path.dirname(ad))}")
        root.geometry("1400x900")
        self.photos=[]
        menu=tk.Menu(root)
        menu.add_command(label="Map Editor", command=lambda: MapEditor(root,ad))
        menu.add_command(label="Sprite Editor", command=lambda: SpriteEditor(root,ad))
        root.config(menu=menu)
        nb=ttk.Notebook(root); nb.pack(fill=tk.BOTH,expand=True,padx=5,pady=5)
        self._pokemon(nb); self._trainers(nb); self._overworld(nb)
        self._palettes(nb); self._data(nb); self._music(nb)

    def _p(self,*a): return os.path.join(self.ad,*a)

    # ---- Pokemon ----
    def _pokemon(self, nb):
        f=ttk.Frame(nb); nb.add(f,text="Pokemon")
        left=ttk.Frame(f); left.pack(side=tk.LEFT,fill=tk.Y,padx=5,pady=5)
        self.pk_list=tk.Listbox(left,width=25,font=("Consolas",9))
        self.pk_list.pack(fill=tk.BOTH,expand=True)
        self.pk_list.bind('<<ListboxSelect>>',lambda e:self._show_pk())
        right=ttk.Frame(f); right.pack(side=tk.LEFT,fill=tk.BOTH,expand=True,padx=5)
        cf=ttk.Frame(right); cf.pack(anchor='w')
        self.pk_shiny=tk.BooleanVar(value=False)
        ttk.Checkbutton(cf,text="Shiny",variable=self.pk_shiny,command=self._show_pk).pack(side=tk.LEFT)
        ttk.Button(cf,text="Import Sprite...",command=self._import_pk).pack(side=tk.LEFT,padx=10)
        self.pk_canvas=tk.Canvas(right,bg='#2a2a2a'); self.pk_canvas.pack(fill=tk.BOTH,expand=True)
        pd=self._p('pokemon')
        self.pk_dirs=sorted([d for d in os.listdir(pd) if os.path.isdir(os.path.join(pd,d))]) if os.path.isdir(pd) else []
        for d in self.pk_dirs: self.pk_list.insert(tk.END,d)

    def _show_pk(self):
        sel=self.pk_list.curselection()
        if not sel: return
        name=self.pk_dirs[sel[0]]; base=self._p('pokemon',name)
        self.pk_canvas.delete('all'); self.photos.clear()
        shiny=self.pk_shiny.get()
        x=10
        for fname,label,wt in [('front','Front',8),('back','Back',8),('icon','Icon',4)]:
            bpp=os.path.join(base,f'{fname}.4bpp')
            png=os.path.join(base,f'{fname}.png')
            pal_file='shiny.gbapal' if shiny else 'normal.gbapal'
            pp=os.path.join(base,pal_file)
            if not os.path.exists(pp): pp=os.path.join(base,'normal.gbapal')
            if os.path.exists(bpp) and os.path.exists(pp):
                pal=read_gbapal(pp); data=open(bpp,'rb').read()
                img=read_4bpp(data,pal,wt)
                img=img.resize((img.width*3,img.height*3),Image.NEAREST)
                p=ImageTk.PhotoImage(img); self.photos.append(p)
                lbl=f'{label} (shiny)' if shiny else label
                self.pk_canvas.create_text(x,10,text=lbl,fill='white',anchor='nw')
                self.pk_canvas.create_image(x,30,anchor='nw',image=p)
                x+=img.width+20
            elif os.path.exists(png):
                img=Image.open(png).convert('RGBA')
                img=img.resize((img.width*3,img.height*3),Image.NEAREST)
                p=ImageTk.PhotoImage(img); self.photos.append(p)
                self.pk_canvas.create_text(x,10,text=label,fill='white',anchor='nw')
                self.pk_canvas.create_image(x,30,anchor='nw',image=p)
                x+=img.width+20

    def _import_pk(self):
        sel=self.pk_list.curselection()
        if not sel: messagebox.showinfo("Info","Select a Pokemon first"); return
        src=filedialog.askopenfilename(filetypes=[("Images","*.png *.bmp *.4bpp")])
        if not src: return
        name=self.pk_dirs[sel[0]]; base=self._p('pokemon',name)
        import shutil
        dst=os.path.join(base,os.path.basename(src))
        shutil.copy2(src,dst)
        messagebox.showinfo("Imported",f"Saved to {dst}")
        self._show_pk()

    # ---- Trainers ----
    def _trainers(self, nb):
        f=ttk.Frame(nb); nb.add(f,text="Trainers")
        left=ttk.Frame(f); left.pack(side=tk.LEFT,fill=tk.Y,padx=5,pady=5)
        self.tr_list=tk.Listbox(left,width=30,font=("Consolas",9))
        self.tr_list.pack(fill=tk.BOTH,expand=True)
        self.tr_list.bind('<<ListboxSelect>>',lambda e:self._show_tr())
        right=ttk.Frame(f); right.pack(side=tk.LEFT,fill=tk.BOTH,expand=True)
        self.tr_canvas=tk.Canvas(right,bg='#2a2a2a'); self.tr_canvas.pack(fill=tk.BOTH,expand=True)
        fp=self._p('trainers','front_pics')
        self.tr_files=sorted([f for f in os.listdir(fp) if f.endswith('.png')]) if os.path.isdir(fp) else []
        for f in self.tr_files: self.tr_list.insert(tk.END,f.replace('_front_pic.png',''))

    def _show_tr(self):
        sel=self.tr_list.curselection()
        if not sel: return
        fp=self._p('trainers','front_pics',self.tr_files[sel[0]])
        self.tr_canvas.delete('all'); self.photos.clear()
        if os.path.exists(fp):
            img=Image.open(fp).convert('RGBA')
            img=img.resize((img.width*4,img.height*4),Image.NEAREST)
            p=ImageTk.PhotoImage(img); self.photos.append(p)
            self.tr_canvas.create_image(10,10,anchor='nw',image=p)

    # ---- Overworld ----
    def _overworld(self, nb):
        f=ttk.Frame(nb); nb.add(f,text="Overworld")
        left=ttk.Frame(f); left.pack(side=tk.LEFT,fill=tk.Y,padx=5,pady=5)
        self.ow_list=tk.Listbox(left,width=25,font=("Consolas",9))
        self.ow_list.pack(fill=tk.BOTH,expand=True)
        self.ow_list.bind('<<ListboxSelect>>',lambda e:self._show_ow())
        right=ttk.Frame(f); right.pack(side=tk.LEFT,fill=tk.BOTH,expand=True)
        self.ow_canvas=tk.Canvas(right,bg='#2a2a2a'); self.ow_canvas.pack(fill=tk.BOTH,expand=True)
        pp=self._p('overworld','pics','people')
        self.ow_files=sorted([f for f in os.listdir(pp) if f.endswith('.png')]) if os.path.isdir(pp) else []
        for f in self.ow_files: self.ow_list.insert(tk.END,f.replace('.png',''))

    def _show_ow(self):
        sel=self.ow_list.curselection()
        if not sel: return
        fp=self._p('overworld','pics','people',self.ow_files[sel[0]])
        self.ow_canvas.delete('all'); self.photos.clear()
        if os.path.exists(fp):
            img=Image.open(fp).convert('RGBA')
            img=img.resize((img.width*4,img.height*4),Image.NEAREST)
            p=ImageTk.PhotoImage(img); self.photos.append(p)
            self.ow_canvas.create_image(10,10,anchor='nw',image=p)

    # ---- Palettes ----
    def _palettes(self, nb):
        f=ttk.Frame(nb); nb.add(f,text="Palettes")
        left=ttk.Frame(f); left.pack(side=tk.LEFT,fill=tk.Y,padx=5,pady=5)
        sf=ttk.Frame(left); sf.pack(fill=tk.X)
        self.pal_s=tk.StringVar(); self.pal_s.trace_add('write',lambda*a:self._filt_pal())
        ttk.Entry(sf,textvariable=self.pal_s,width=25).pack(fill=tk.X)
        self.pal_list=tk.Listbox(left,width=35,font=("Consolas",8))
        self.pal_list.pack(fill=tk.BOTH,expand=True)
        self.pal_list.bind('<<ListboxSelect>>',lambda e:self._show_pal())
        right=ttk.Frame(f); right.pack(side=tk.LEFT,fill=tk.BOTH,expand=True,padx=5)
        self.pal_cv=tk.Canvas(right,bg='#2a2a2a',height=80); self.pal_cv.pack(fill=tk.X,pady=5)
        self.pal_cv.bind('<Button-1>',self._edit_pal)
        ttk.Button(right,text="Save",command=self._save_pal).pack(anchor='w')
        self.pal_info=tk.StringVar(value="Click color to edit"); ttk.Label(right,textvariable=self.pal_info).pack(anchor='w')
        self.all_pals=[]; self.cur_pp=None; self.cur_pc=None
        for r,d,fs in os.walk(self.ad):
            for fn in fs:
                if fn.endswith('.gbapal'): self.all_pals.append(os.path.relpath(os.path.join(r,fn),self.ad))
        self.all_pals.sort(); self.filt_p=list(self.all_pals)
        for p in self.filt_p: self.pal_list.insert(tk.END,p)

    def _filt_pal(self):
        s=self.pal_s.get().lower(); self.pal_list.delete(0,tk.END)
        self.filt_p=[p for p in self.all_pals if s in p.lower()]
        for p in self.filt_p: self.pal_list.insert(tk.END,p)

    def _show_pal(self):
        sel=self.pal_list.curselection()
        if not sel: return
        self.cur_pp=self._p(self.filt_p[sel[0]]); self.cur_pc=read_gbapal(self.cur_pp); self._draw_pal()

    def _draw_pal(self):
        self.pal_cv.delete('all')
        if not self.cur_pc: return
        for i,(r,g,b) in enumerate(self.cur_pc):
            x=i*40+10
            self.pal_cv.create_rectangle(x,10,x+38,48,fill=f'#{r:02x}{g:02x}{b:02x}',outline='white')
            self.pal_cv.create_text(x+19,58,text=str(i),fill='white',font=("Consolas",8))

    def _edit_pal(self, ev):
        if not self.cur_pc: return
        i=(ev.x-10)//40
        if i<0 or i>=len(self.cur_pc): return
        c=colorchooser.askcolor(initialcolor='#{:02x}{:02x}{:02x}'.format(*self.cur_pc[i]))
        if c[0]: self.cur_pc[i]=tuple(int(v) for v in c[0]); self._draw_pal()

    def _save_pal(self):
        if self.cur_pp and self.cur_pc: write_gbapal(self.cur_pp,self.cur_pc); self.pal_info.set("Saved!")

    # ---- Data ----
    def _data(self, nb):
        f=ttk.Frame(nb); nb.add(f,text="Data")
        sub=ttk.Notebook(f); sub.pack(fill=tk.BOTH,expand=True,padx=5,pady=5)
        for dt in ['pokemon','moves']:
            sf=ttk.Frame(sub); sub.add(sf,text=dt.capitalize())
            jp=self._p('data',f'{dt}.json')
            if not os.path.exists(jp): ttk.Label(sf,text=f"No {dt}.json").pack(); continue
            try: data=json.load(open(jp))
            except: ttk.Label(sf,text=f"Error loading").pack(); continue
            if not data: continue
            cols=list(data[0].keys())
            tree=ttk.Treeview(sf,columns=cols,show='headings',height=25)
            for c in cols: tree.heading(c,text=c); tree.column(c,width=80 if c!='name' else 120)
            vs=ttk.Scrollbar(sf,orient=tk.VERTICAL,command=tree.yview)
            tree.config(yscrollcommand=vs.set); vs.pack(side=tk.RIGHT,fill=tk.Y)
            tree.pack(fill=tk.BOTH,expand=True)
            for e in data: tree.insert('',tk.END,values=[e.get(c,'') for c in cols])
            def mk(tree,cols,data,jp):
                def ed(ev):
                    it=tree.selection()
                    if not it: return
                    ci=int(tree.identify_column(ev.x).replace('#',''))-1
                    if ci<0: return
                    cn=cols[ci]; cur=tree.item(it[0])['values'][ci]
                    pop=tk.Toplevel(self.root); pop.title(f"Edit {cn}"); pop.geometry("300x100")
                    sv=tk.StringVar(value=str(cur))
                    e=ttk.Entry(pop,textvariable=sv,width=30); e.pack(padx=10,pady=10); e.select_range(0,tk.END); e.focus()
                    def save():
                        v=sv.get()
                        try: v=int(v)
                        except: pass
                        vals=list(tree.item(it[0])['values']); vals[ci]=v; tree.item(it[0],values=vals)
                        data[tree.index(it[0])][cn]=v; json.dump(data,open(jp,'w'),indent=2); pop.destroy()
                    ttk.Button(pop,text="Save",command=save).pack(pady=5); pop.bind('<Return>',lambda e:save())
                tree.bind('<Double-1>',ed)
            mk(tree,cols,data,jp)

    # ---- Music ----
    def _music(self, nb):
        f=ttk.Frame(nb); nb.add(f,text="Music")
        left=ttk.Frame(f); left.pack(side=tk.LEFT,fill=tk.Y,padx=5,pady=5)
        self.mu_list=tk.Listbox(left,width=35,font=("Consolas",9))
        self.mu_list.pack(fill=tk.BOTH,expand=True)
        right=ttk.Frame(f); right.pack(side=tk.LEFT,fill=tk.BOTH,expand=True,padx=5)
        bf=ttk.Frame(right); bf.pack(pady=10)
        ttk.Button(bf,text="▶ Play",command=self._play).pack(side=tk.LEFT,padx=5)
        ttk.Button(bf,text="⏹ Stop",command=self._stop).pack(side=tk.LEFT,padx=5)
        ttk.Button(bf,text="Import MIDI...",command=self._imp_mus).pack(side=tk.LEFT,padx=5)
        self.mu_info=tk.StringVar(value="Select a song"); ttk.Label(right,textvariable=self.mu_info,font=("Consolas",10)).pack(pady=10)
        self.mu_files=[]
        md=self._p('sound','songs','midi')
        if os.path.isdir(md):
            for fn in sorted(os.listdir(md)):
                if fn.endswith(('.mid','.ogg','.wav','.mp3')): self.mu_files.append(fn)
        for f in self.mu_files: self.mu_list.insert(tk.END,f)
        self._mx=False

    def _play(self):
        sel=self.mu_list.curselection()
        if not sel: return
        fp=self._p('sound','songs','midi',self.mu_files[sel[0]])
        try:
            import pygame
            if not self._mx: pygame.mixer.init(44100,-16,2,2048); self._mx=True
            pygame.mixer.music.load(fp); pygame.mixer.music.play()
            self.mu_info.set(f"Playing: {self.mu_files[sel[0]]}")
        except ImportError: self.mu_info.set("pip install pygame")
        except Exception as e: self.mu_info.set(str(e))

    def _stop(self):
        try: import pygame; pygame.mixer.music.stop(); self.mu_info.set("Stopped")
        except: pass

    def _imp_mus(self):
        src=filedialog.askopenfilename(filetypes=[("MIDI","*.mid")])
        if not src: return
        import shutil
        td=self._p('sound','songs','midi'); os.makedirs(td,exist_ok=True)
        shutil.copy2(src,os.path.join(td,os.path.basename(src)))
        messagebox.showinfo("Done",f"Imported: {os.path.basename(src)}")


# ---- Map Editor Window ----
class MapEditor:
    def __init__(self, parent, ad):
        self.ad=ad; self.win=tk.Toplevel(parent)
        self.win.title("Map Editor"); self.win.geometry("1200x800")
        self.photos=[]; self.mdata=None; self.mw=0; self.mh=0; self.mpath=None; self.scale=6

        left=ttk.Frame(self.win); left.pack(side=tk.LEFT,fill=tk.Y,padx=5,pady=5)
        ttk.Label(left,text="Maps:").pack(anchor='w')
        self.mlist=tk.Listbox(left,width=25,font=("Consolas",9))
        self.mlist.pack(fill=tk.BOTH,expand=True)
        self.mlist.bind('<<ListboxSelect>>',lambda e:self._load())

        right=ttk.Frame(self.win); right.pack(side=tk.LEFT,fill=tk.BOTH,expand=True,padx=5,pady=5)
        self.info=tk.StringVar(value="Select a map"); ttk.Label(right,textvariable=self.info,font=("Consolas",9)).pack(anchor='w')
        tool=ttk.Frame(right); tool.pack(fill=tk.X,pady=2)
        ttk.Label(tool,text="Tile:").pack(side=tk.LEFT)
        self.tid=tk.IntVar(value=1)
        ttk.Spinbox(tool,from_=0,to=1023,textvariable=self.tid,width=6).pack(side=tk.LEFT,padx=5)
        ttk.Button(tool,text="Save",command=self._save).pack(side=tk.LEFT,padx=10)

        cf=ttk.Frame(right); cf.pack(fill=tk.BOTH,expand=True)
        self.cv=tk.Canvas(cf,bg='#1a1a1a')
        sy=ttk.Scrollbar(cf,orient=tk.VERTICAL,command=self.cv.yview)
        sx=ttk.Scrollbar(cf,orient=tk.HORIZONTAL,command=self.cv.xview)
        self.cv.config(yscrollcommand=sy.set,xscrollcommand=sx.set)
        sy.pack(side=tk.RIGHT,fill=tk.Y); sx.pack(side=tk.BOTTOM,fill=tk.X)
        self.cv.pack(fill=tk.BOTH,expand=True)
        self.cv.bind('<Button-1>',self._paint); self.cv.bind('<B1-Motion>',self._paint)

        # Load layouts
        ld=os.path.join(ad,'layouts')
        self.layouts=[]
        lj=os.path.join(ld,'layouts.json')
        if os.path.exists(lj):
            try:
                d=json.load(open(lj))
                self.layouts=d.get('layouts',[])
            except: pass
        for l in self.layouts:
            self.mlist.insert(tk.END, l.get('name','?').replace('_Layout',''))

    def _load(self):
        sel=self.mlist.curselection()
        if not sel: return
        l=self.layouts[sel[0]]
        self.mw=l.get('width',0); self.mh=l.get('height',0)
        bp=l.get('blockdata_filepath','')
        # Try extracted layouts dir first
        name=l.get('name','').replace('_Layout','')
        mp=os.path.join(self.ad,'layouts',name,'map.bin')
        if not os.path.exists(mp) and bp:
            # Try relative to parent
            for root in ['.','..',os.path.dirname(os.path.dirname(self.ad))]:
                tp=os.path.join(root,bp)
                if os.path.exists(tp): mp=tp; break
        if not os.path.exists(mp) or self.mw<=0 or self.mh<=0:
            self.info.set(f"{name}: no map data"); self.mdata=None; return
        raw=open(mp,'rb').read()
        exp=self.mw*self.mh*2
        if len(raw)<exp: self.info.set(f"{name}: data too small"); return
        self.mdata=list(struct.unpack_from(f'<{self.mw*self.mh}H',raw,0))
        self.mpath=mp
        self.info.set(f"{name}: {self.mw}x{self.mh} — click to paint")
        self._render()

    def _render(self):
        self.cv.delete('all'); self.photos.clear()
        if not self.mdata: return
        s=self.scale; w,h=self.mw,self.mh
        img=Image.new('RGB',(w*s,h*s))
        px=img.load()
        for y in range(h):
            for x in range(w):
                mt=self.mdata[y*w+x]&0x3FF
                r,g,b=(mt*7)&0xFF,(mt*13)&0xFF,(mt*23)&0xFF
                for dy in range(s):
                    for dx in range(s):
                        px[x*s+dx,y*s+dy]=(r,g,b)
        p=ImageTk.PhotoImage(img); self.photos.append(p)
        self.cv.create_image(0,0,anchor='nw',image=p)
        self.cv.config(scrollregion=(0,0,w*s,h*s))

    def _paint(self, ev):
        if not self.mdata: return
        x,y=int(self.cv.canvasx(ev.x))//self.scale, int(self.cv.canvasy(ev.y))//self.scale
        if x<0 or x>=self.mw or y<0 or y>=self.mh: return
        old=self.mdata[y*self.mw+x]
        self.mdata[y*self.mw+x]=(old&0xFC00)|self.tid.get()&0x3FF
        self._render()

    def _save(self):
        if not self.mdata or not self.mpath: return
        open(self.mpath,'wb').write(struct.pack(f'<{len(self.mdata)}H',*self.mdata))
        self.info.set("Saved!")


# ---- Sprite Editor Window ----
class SpriteEditor:
    def __init__(self, parent, ad):
        self.ad=ad; self.win=tk.Toplevel(parent)
        self.win.title("Sprite Editor"); self.win.geometry("800x600")
        self.img=None; self.photo=None; self.zoom=8; self.tool='pen'; self.color=(255,255,255,255)
        self.layers=[]; self.active_layer=0

        menu=tk.Menu(self.win)
        fmenu=tk.Menu(menu,tearoff=0)
        fmenu.add_command(label="Open PNG...",command=self._open)
        fmenu.add_command(label="Import from game...",command=self._import)
        fmenu.add_command(label="Save",command=self._save)
        fmenu.add_command(label="Save As...",command=self._save_as)
        fmenu.add_command(label="Export to game asset...",command=self._export)
        menu.add_cascade(label="File",menu=fmenu)
        self.win.config(menu=menu)

        top=ttk.Frame(self.win); top.pack(fill=tk.X,padx=5,pady=5)
        ttk.Button(top,text="Pen",command=lambda:self._set_tool('pen')).pack(side=tk.LEFT,padx=2)
        ttk.Button(top,text="Eraser",command=lambda:self._set_tool('eraser')).pack(side=tk.LEFT,padx=2)
        ttk.Button(top,text="Color",command=self._pick_color).pack(side=tk.LEFT,padx=2)
        ttk.Label(top,text="Zoom:").pack(side=tk.LEFT,padx=5)
        self.zoom_var=tk.IntVar(value=8)
        for z in [4,8,16]:
            ttk.Radiobutton(top,text=f"{z}x",value=z,variable=self.zoom_var,command=self._redraw).pack(side=tk.LEFT)

        ttk.Label(top,text="  Layer:").pack(side=tk.LEFT,padx=5)
        self.layer_var=tk.IntVar(value=0)
        ttk.Spinbox(top,from_=0,to=9,textvariable=self.layer_var,width=3,command=self._switch_layer).pack(side=tk.LEFT)
        ttk.Button(top,text="+Layer",command=self._add_layer).pack(side=tk.LEFT,padx=5)
        ttk.Label(top,text="Opacity:").pack(side=tk.LEFT)
        self.opacity=tk.IntVar(value=255)
        ttk.Scale(top,from_=0,to=255,variable=self.opacity,orient=tk.HORIZONTAL,length=100,command=lambda v:self._redraw()).pack(side=tk.LEFT)

        self.cv=tk.Canvas(self.win,bg='#333'); self.cv.pack(fill=tk.BOTH,expand=True,padx=5,pady=5)
        self.cv.bind('<Button-1>',self._draw); self.cv.bind('<B1-Motion>',self._draw)
        self.save_path=None
        self.tool_label=tk.StringVar(value="Tool: pen")
        ttk.Label(self.win,textvariable=self.tool_label).pack(anchor='w',padx=5)

    def _set_tool(self,t): self.tool=t; self.tool_label.set(f"Tool: {t}")
    def _pick_color(self):
        c=colorchooser.askcolor()
        if c[0]: self.color=(*[int(v) for v in c[0]],255)

    def _open(self):
        fp=filedialog.askopenfilename(filetypes=[("Images","*.png")])
        if not fp: return
        self.layers=[Image.open(fp).convert('RGBA')]
        self.active_layer=0; self.save_path=fp; self._redraw()

    def _import(self):
        fp=filedialog.askopenfilename(initialdir=self.ad,filetypes=[("Images","*.png *.4bpp")])
        if not fp: return
        if fp.endswith('.png'):
            self.layers=[Image.open(fp).convert('RGBA')]
        else:
            data=open(fp,'rb').read()
            self.layers=[read_4bpp(data,GRAY,8)]
        self.active_layer=0; self.save_path=fp; self._redraw()

    def _save(self):
        if not self.layers: return
        if not self.save_path: self._save_as(); return
        self._flatten().save(self.save_path)

    def _save_as(self):
        if not self.layers: return
        fp=filedialog.asksaveasfilename(defaultextension=".png",filetypes=[("PNG","*.png")])
        if fp: self.save_path=fp; self._flatten().save(fp)

    def _export(self):
        if not self.layers: return
        fp=filedialog.asksaveasfilename(initialdir=self.ad,defaultextension=".png",filetypes=[("PNG","*.png")])
        if fp: self._flatten().save(fp); messagebox.showinfo("Exported",f"Saved: {fp}")

    def _add_layer(self):
        if not self.layers: return
        w,h=self.layers[0].size
        self.layers.append(Image.new('RGBA',(w,h),(0,0,0,0)))
        self.active_layer=len(self.layers)-1
        self.layer_var.set(self.active_layer)

    def _switch_layer(self):
        self.active_layer=min(self.layer_var.get(),len(self.layers)-1)

    def _flatten(self):
        if not self.layers: return Image.new('RGBA',(1,1))
        base=Image.new('RGBA',self.layers[0].size,(0,0,0,0))
        for l in self.layers: base=Image.alpha_composite(base,l)
        return base

    def _redraw(self):
        self.cv.delete('all')
        if not self.layers: return
        z=self.zoom_var.get()
        flat=self._flatten()
        disp=flat.resize((flat.width*z,flat.height*z),Image.NEAREST)
        self.photo=ImageTk.PhotoImage(disp)
        self.cv.create_image(0,0,anchor='nw',image=self.photo)

    def _draw(self, ev):
        if not self.layers or self.active_layer>=len(self.layers): return
        z=self.zoom_var.get()
        x,y=ev.x//z, ev.y//z
        layer=self.layers[self.active_layer]
        if x<0 or y<0 or x>=layer.width or y>=layer.height: return
        if self.tool=='pen':
            a=self.opacity.get()
            c=(self.color[0],self.color[1],self.color[2],a)
            layer.putpixel((x,y),c)
        elif self.tool=='eraser':
            layer.putpixel((x,y),(0,0,0,0))
        self._redraw()


def main():
    ad=ASSET_DIR
    if len(sys.argv)>1: ad=sys.argv[1]
    if not os.path.isdir(ad):
        ed=os.path.dirname(os.path.abspath(sys.argv[0]))
        ad=os.path.join(ed,ASSET_DIR)
    if not os.path.isdir(ad):
        root=tk.Tk(); root.withdraw()
        ad=filedialog.askdirectory(title="Select extracted_assets/frlg folder")
        if not ad: sys.exit(0); root.destroy()
    root=tk.Tk(); PkmnEditor(root,ad); root.mainloop()

if __name__=='__main__': main()
