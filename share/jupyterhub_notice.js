//
// jupyterhub_notice.js - JupyterLab simple notice watcher
//
//
//

${NB_WRKDIR}/.jupyterhub_notice.txt

// notice.js - JupyterLab simple notice watcher
(() => {
  // home directory
  const base = document.body.getAttribute('data-base-url') || '/';
  const url  = base + '.jupyterhub_notice.txt';
  let last = '';

  async function poll(){
    try{
      const r = await fetch(url + '?_=' + Date.now());
      if(!r.ok) return;
      const t = await r.text();
      if(t && t !== last){
        last = t;
        toast(t);
      }
    }catch(e){}
  }

  function toast(msg){
    const el = document.createElement('div');
    el.textContent = msg;
    el.style = 'position:fixed;right:20px;bottom:20px;background:#ffefc6;color:#222;padding:10px 16px;border-radius:10px;box-shadow:0 2px 8px rgba(0,0,0,.2);z-index:9999;max-width:360px;';
    document.body.appendChild(el);
    setTimeout(()=>el.remove(),15000);
  }

  setTimeout(()=>{ poll(); setInterval(poll,30000); }, 1000);
})();


