//
// src/index.ts  (jnotice.js)
//   ver 1.40  sticky 空／削除(404/410)を検知して表示中の無限トーストを自動消去．
//

import { JupyterFrontEnd, JupyterFrontEndPlugin } from '@jupyterlab/application';
import { PageConfig } from '@jupyterlab/coreutils';

const DEF_POLL_MS = 60_000;                 // デフォルト ポーリング間隔（ミリ秒）
const NOTICE_PATH = '.jnotice.txt';         // 通常通知ファイル（表示後に削除）
const STICKY_PATH = '.jnotice.sticky.txt';  // 無限表示ファイル（削除しない）

type JNoticeCfg = { path?: string; pollMs?: number; stickyPath?: string };

const plugin: JupyterFrontEndPlugin<void> = {
  id: 'jnotice:plugin',
  autoStart: true,
  activate: async (_app: JupyterFrontEnd) => {
    // ---- 設定取得 ----
    const cfgEl = document.getElementById('jupyter-config-data');
    const raw   = cfgEl?.textContent ?? '';
    const cfg   = raw ? JSON.parse(raw) : {};

    const jc = (
      (cfg as any).jnotice ??
      (cfg as any).page_config_data?.jnotice ??
      (cfg as any).pageConfig?.jnotice ??
      {}
    ) as JNoticeCfg;

    // baseUrl は PageConfig を優先，無ければパスから推定．
    const baseFromJL  = PageConfig.getBaseUrl?.() ?? '';
    const baseFromLoc = (location.pathname.match(/\/user\/[^/]+\/?/)?.[0] || '/').replace(/\/?$/, '/');
    const base: string = ( (cfg as any).baseUrl || baseFromJL || baseFromLoc ).replace(/\/?$/, '/');

    // /hub/user/<name>/... に迷い込んだ場合は /user/<name>/ に矯正．location が取得できるなら最優先．
    let fixedBase = base;
    const hubUserMatch = fixedBase.match(/^\/hub\/(user\/[^/]+\/)/);
    if (hubUserMatch) fixedBase = '/' + hubUserMatch[1];
    const locUser = (location.pathname.match(/\/user\/[^/]+\/?/)?.[0] || '').replace(/\/?$/, '/');
    if (locUser) fixedBase = locUser;

    // 監視ファイルパス
    const noticePath = (typeof jc.path === 'string' && jc.path.trim()) ? jc.path : NOTICE_PATH;
    const stickyPath = (typeof jc.stickyPath === 'string' && jc.stickyPath.trim()) ? jc.stickyPath : STICKY_PATH;

    const pollMs     = Number.isFinite(jc.pollMs as any) ? Number(jc.pollMs) : DEF_POLL_MS;

    // XSRF
    const xsrf = (document.cookie.match(/(?:^|; )_xsrf=([^;]+)/)?.[1] || '');

    // 取得 URL（/files/...）
    const filesUrl  = fixedBase + 'files/' + noticePath.replace(/^\/+/, '');
    const stickyUrl = fixedBase + 'files/' + stickyPath.replace(/^\/+/, '');

    // ネイティブ API を捕獲（他スクリプトのモンキーパッチ回避）．
    const nativeSetTimeout   = window.setTimeout.bind(window);
    const nativeClearTimeout = window.clearTimeout.bind(window);
    const nativeFetch        = window.fetch.bind(window);

    // 状態
    let timer: number | undefined;
    let lastSticky = '';                            // sticky の直近内容
    let stickyEl: HTMLDivElement | null = null;     // 表示中の sticky 要素参照

    // ---- フェード共通 ----
    function fadeAndRemove(el: HTMLElement) {
      el.style.opacity = '0';
      const removeAfter = () => el.remove();
      el.addEventListener('transitionend', removeAfter, { once: true });
      nativeSetTimeout(removeAfter, 350); // フォールバック
    }

    // ---- UI: 通常トースト（60s 自動フェード削除） ----
    function toast(msg: string) {
      const el = document.createElement('div');
      el.setAttribute('role', 'status');
      el.setAttribute('aria-live', 'polite');
      el.style.cssText = [
        'position:fixed','right:20px','bottom:20px',
        'background:#ffefc6','color:#222',
        'padding:10px 14px','border-radius:10px',
        'box-shadow:0 2px 8px rgba(0,0,0,.2)',
        'z-index:9999','max-width:360px',
        'display:flex','align-items:center','gap:8px',
        'opacity:1','transition:opacity 300ms ease'
      ].join(';') + ';';

      const text = document.createElement('span');
      text.textContent = msg;
      text.style.cssText = 'line-height:1.4;';
      el.appendChild(text);

      const btn = document.createElement('button');
      btn.type = 'button';
      btn.setAttribute('aria-label', 'close');
      btn.textContent = '×';
      btn.style.cssText = [
        'border:none','background:transparent','font-size:16px','cursor:pointer',
        'line-height:1','padding:4px','margin-left:auto','border-radius:6px'
      ].join(';') + ';';
      btn.onmouseenter = () => (btn.style.backgroundColor = 'rgba(0,0,0,.06)');
      btn.onmouseleave = () => (btn.style.backgroundColor = 'transparent');
      btn.onclick = () => fadeAndRemove(el);

      el.appendChild(btn);
      document.body.appendChild(el);

      // 60 秒後に自動フェード→削除
      nativeSetTimeout(() => fadeAndRemove(el), 60_000);
    }

    // ---- UI: 無限トースト（手動クローズのみ） ----
    function toastSticky(msg: string): HTMLDivElement {
      const el = document.createElement('div');
      el.setAttribute('role', 'status');
      el.setAttribute('aria-live', 'polite');
      el.style.cssText = [
        'position:fixed','right:20px','bottom:20px',
        'background:#cfe9ff','color:#122',
        'padding:12px 16px','border-left:6px solid #2b6cb0',
        'border-radius:10px','box-shadow:0 2px 8px rgba(0,0,0,.2)',
        'z-index:9999','max-width:420px','display:flex',
        'align-items:flex-start','gap:10px',
        'opacity:1','transition:opacity 300ms ease'
      ].join(';') + ';';

      const text = document.createElement('span');
      text.textContent = msg;
      text.style.cssText = 'line-height:1.5;';
      el.appendChild(text);

      const btn = document.createElement('button');
      btn.type = 'button';
      btn.setAttribute('aria-label', 'close');
      btn.textContent = '×';
      btn.style.cssText = [
        'border:none','background:transparent','font-size:18px','cursor:pointer',
        'line-height:1','padding:4px','margin-left:auto','border-radius:6px'
      ].join(';') + ';';
      btn.onmouseenter = () => (btn.style.backgroundColor = 'rgba(0,0,0,.06)');
      btn.onmouseleave = () => (btn.style.backgroundColor = 'transparent');
      btn.onclick = () => { fadeAndRemove(el); stickyEl = null; };

      el.appendChild(btn);
      document.body.appendChild(el);
      return el; // 参照を返す
    }

    // ---- Helper: 通常通知ファイルの削除（失敗時は空 PUT） ----
    async function deleteNoticeFile(): Promise<void> {
      const apiPath = noticePath.replace(/^\/+/, '').split('/').map(encodeURIComponent).join('/');
      const apiUrl  = fixedBase + 'api/contents/' + apiPath;

      try {
        const delHeaders: Record<string, string> = { 'Content-Type': 'application/json' };
        if (xsrf) delHeaders['X-XSRFToken'] = xsrf;
        const del = await nativeFetch(apiUrl, {
          method: 'DELETE',
          credentials: 'same-origin',
          headers: delHeaders
        });
        if (del.ok || del.status === 204) return;
      } catch { /* Skip */ }

      try {
        const putHeaders: Record<string, string> = { 'Content-Type': 'application/json' };
        if (xsrf) putHeaders['X-XSRFToken'] = xsrf;
        await nativeFetch(apiUrl, {
          method: 'PUT',
          credentials: 'same-origin',
          headers: putHeaders,
          body: JSON.stringify({ type: 'file', format: 'text', content: '' })
        });
      } catch { /* Skip */ }
    }

    // ---- 1 回分のポーリング：sticky を優先．両方本文ありなら通常は表示せず削除．sticky 変更・空・削除を反映 ----
    async function pollOnce() {
      try {
        const h: Record<string, string> = {};
        if (xsrf) h['X-XSRFToken'] = xsrf;

        // === 1) 先に sticky（無限表示）を確認 ===
        let stickyActive = false;

        try {
          const rs = await nativeFetch(stickyUrl + '?_=' + Date.now(), {
            cache: 'no-store', credentials: 'same-origin', headers: h
          });

          if (rs.status !== 424) {
            const sFinal = rs.url || '';
            const sType  = (rs.headers.get('Content-Type') || '').toLowerCase();
            const diverted = rs.redirected || /\/hub\/login|oauth_callback/.test(sFinal) || sType.includes('text/html');

            if (!diverted && rs.ok) {
              const s = (await rs.text()).trim();
              if (s) {
                stickyActive = true;
                if (s !== lastSticky) {
                  if (stickyEl) { fadeAndRemove(stickyEl); stickyEl = null; }
                  stickyEl = toastSticky(s);
                  lastSticky = s;
                }
              } else {
                // 空になったら表示中を消す
                if (stickyEl) { fadeAndRemove(stickyEl); stickyEl = null; }
                lastSticky = '';
              }
            } else if (rs.status === 404 || rs.status === 410) {
              // ★ ファイル削除を空扱い：表示中なら消す
              if (stickyEl) { fadeAndRemove(stickyEl); stickyEl = null; }
              lastSticky = '';
            }
          }
        } catch { /* sticky 取得失敗は黙って次へ */ }

        // === 2) 通常通知（この周期で sticky があれば表示せず削除のみ） ===
        try {
          const r = await nativeFetch(filesUrl + '?_=' + Date.now(), {
            cache: 'no-store', credentials: 'same-origin', headers: h
          });

          if (r.status === 424) return; // ユーザサーバ未到達→次 tick

          const finalURL = r.url || '';
          const ctype = (r.headers.get('Content-Type') || '').toLowerCase();
          const diverted = r.redirected || /\/hub\/login|oauth_callback/.test(finalURL) || ctype.includes('text/html');

          if (!diverted && r.ok) {
            const t = (await r.text()).trim();
            if (!t) return;

            if (stickyActive) {
              // 両方本文あり：通常は表示せず即削除
              await deleteNoticeFile();
            } else {
              // 通常どおり表示→削除
              toast(t);
              await deleteNoticeFile();
            }
          }
        } catch { /* 通常通知取得失敗は黙って次へ */ }

      } catch {
        // ネットワーク失敗時は次の tick で自己復旧
      }
    }

    // ---- 反復：自己再帰 setTimeout（止まりにくい）----
    function schedule() {
      timer = nativeSetTimeout(async () => {
        try { await pollOnce(); }
        finally { schedule(); }     // 必ず次回を予約（自己復旧）．
      }, pollMs);
    }

    // 初回遅延＋開始
    nativeSetTimeout(() => { void pollOnce(); schedule(); }, 1000);

    // タブ復帰，履歴復帰（BFCache 等）で即チェック
    document.addEventListener('visibilitychange', () => { if (!document.hidden) void pollOnce(); });
    window.addEventListener('pageshow', () => { void pollOnce(); });

    // ページ離脱でクリーンアップ
    window.addEventListener('beforeunload', () => { if (timer) nativeClearTimeout(timer); });

    // 起動ログ
    console.log(`[jnotice] plugin loaded; polling "${noticePath}" and sticky "${stickyPath}" every ${pollMs} ms`);
  }
};

export default plugin;
