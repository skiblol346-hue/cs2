#!/usr/bin/env python3
"""
cs2_ai_rag.py
=============
Interactive Hybrid RAG & Vector Database Query Engine for the CS2 Repository (`skiblol346-hue/cs2`).
Performs Full-Text Search (BM25 via FTS5), Cosine Similarity Vector Search (128D Hashed n-grams + Domain Boosting),
and Hybrid Ranking over the Obsidian Vault & SQLite Vector Database (`cs2_ai_vectordb.sqlite`).

Usage:
  python3 cs2_ai_rag.py search "<query>" [--top-k 5] [--project <project_name>] [--mode {vector,keyword,hybrid}]
  python3 cs2_ai_rag.py ask "<question>" [--top-k 4]
  python3 cs2_ai_rag.py stats
  python3 cs2_ai_rag.py get-chunk <chunk_id>
  python3 cs2_ai_rag.py list-projects
"""

import os
import sys
import argparse
import sqlite3
import math
import struct
import re

REPO_ROOT = "/home/user/cs2"
VAULT_DIR = os.path.join(REPO_ROOT, "obsidian_vault")
DB_PATH = os.path.join(REPO_ROOT, "cs2_ai_vectordb.sqlite")

DOMAIN_KEYWORDS = [
    "aimbot", "esp", "chams", "glow", "triggerbot", "bhop", "autostrafe",
    "netvars", "schema", "offsets", "dumper", "memory", "kernel", "driver",
    "hook", "vmt", "minhook", "createmove", "present", "reset", "imgui",
    "directx", "dx11", "gui", "menu", "entity", "player", "pawn", "controller",
    "weapon", "raytrace", "trace", "matrix", "viewmatrix", "worldtoscreen",
    "skeleton", "hitbox", "bone", "radar", "skin", "inventory", "changer",
    "sdk", "interface", "pattern", "signature", "scan", "bypass", "anticheat"
]

def compute_query_vector(text, dim=128):
    """Compute normalized 128D query vector matching the indexing scheme."""
    vector = [0.0] * dim
    if not text:
        return vector
        
    lower_text = text.lower()
    words = re.findall(r'[a-zA-Z0-9_]{3,}', lower_text)
    for w in words:
        h = hash(w) % dim
        vector[h] += 1.0
        
    for kw in DOMAIN_KEYWORDS:
        if kw in lower_text:
            count = lower_text.count(kw)
            h = hash(kw) % dim
            vector[h] += 5.0 * count
            
    for i in range(len(lower_text) - 3):
        ngram = lower_text[i:i+4]
        h = hash(ngram) % dim
        vector[h] += 0.2
        
    norm = math.sqrt(sum(v * v for v in vector))
    if norm > 1e-6:
        vector = [v / norm for v in vector]
    return vector

def blob_to_vector(blob):
    """Unpack binary BLOB to float list."""
    dim = len(blob) // 4
    return struct.unpack(f'{dim}f', blob)

def cosine_similarity(v1, v2):
    """Compute dot product / cosine similarity of two L2-normalized vectors."""
    return sum(a * b for a, b in zip(v1, v2))

def get_db_connection():
    if not os.path.exists(DB_PATH):
        print(f"Error: Database not found at {DB_PATH}. Please run `python3 build_ai_vault_and_vectordb.py` first.", file=sys.stderr)
        sys.exit(1)
    return sqlite3.connect(DB_PATH)

def run_stats():
    conn = get_db_connection()
    cur = conn.cursor()
    
    print("\n" + "="*60)
    print("      CS2 AI OBSIDIAN VAULT & VECTOR DB STATISTICS      ")
    print("="*60)
    
    cur.execute("SELECT key, value FROM metadata")
    for k, v in cur.fetchall():
        print(f"  - {k:<25}: {v}")
        
    cur.execute("SELECT project, COUNT(*) FROM chunks GROUP BY project ORDER BY COUNT(*) DESC LIMIT 10")
    print("\n  Top 10 Projects by Indexed Chunks:")
    for proj, cnt in cur.fetchall():
        print(f"    * {proj:<38}: {cnt:>5} chunks")
        
    cur.execute("SELECT COUNT(DISTINCT file_path) FROM chunks")
    files_cnt = cur.fetchone()[0]
    print(f"\n  Total Distinct Files Chunked: {files_cnt}")
    print("="*60 + "\n")
    conn.close()

def list_projects():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT project, COUNT(DISTINCT file_path), COUNT(*) FROM chunks GROUP BY project ORDER BY project ASC")
    rows = cur.fetchall()
    
    print("\n" + "="*75)
    print(f" {'Project Name':<42} | {'Files':<8} | {'Chunks':<8} ")
    print("="*75)
    for p, f, c in rows:
        print(f" {p:<42} | {f:<8} | {c:<8} ")
    print("="*75 + "\n")
    conn.close()

def search_db(query, top_k=5, project=None, mode="hybrid"):
    conn = get_db_connection()
    cur = conn.cursor()
    
    # 1. Keyword / BM25 Search using FTS5
    fts_scores = {}
    fts_query = re.sub(r'[^a-zA-Z0-9_\s]', ' ', query).strip()
    if fts_query:
        # Match tokens via FTS5
        tokens = fts_query.split()
        if tokens:
            fts_expr = " OR ".join([f'"{t}"*' for t in tokens])
            try:
                sql = "SELECT rowid, bm25(chunks_fts) FROM chunks_fts WHERE chunks_fts MATCH ?"
                params = [fts_expr]
                if project:
                    sql += " AND project = ?"
                    params.append(project)
                sql += " LIMIT 200"
                cur.execute(sql, params)
                for rowid, score in cur.fetchall():
                    # SQLite bm25 returns negative values, more negative is better match -> normalize
                    fts_scores[rowid] = abs(score) if score != 0 else 1.0
            except Exception as e:
                pass
                
    # Normalize FTS scores to 0.0 - 1.0 range
    if fts_scores:
        max_fts = max(fts_scores.values())
        if max_fts > 0:
            for k in fts_scores:
                fts_scores[k] = min(1.0, fts_scores[k] / max_fts)
                
    # 2. Vector Cosine Similarity Search
    vec_scores = {}
    if mode in ["vector", "hybrid"]:
        q_vec = compute_query_vector(query, dim=128)
        sql = "SELECT e.chunk_id, e.vector FROM embeddings e JOIN chunks c ON c.id = e.chunk_id"
        params = []
        if project:
            sql += " WHERE c.project = ?"
            params.append(project)
        cur.execute(sql, params)
        for cid, blob in cur.fetchall():
            vec = blob_to_vector(blob)
            sim = cosine_similarity(q_vec, vec)
            vec_scores[cid] = max(0.0, sim)
            
    # Normalize Vector scores
    if vec_scores:
        max_vec = max(vec_scores.values()) if max(vec_scores.values()) > 0 else 1.0
        for k in vec_scores:
            vec_scores[k] = vec_scores[k] / max_vec
            
    # Combine & Rank
    all_ids = set(fts_scores.keys()).union(set(vec_scores.keys()))
    if not all_ids and mode == "keyword":
        # Fallback to LIKE substring search if FTS exact match missed
        sql = "SELECT id FROM chunks WHERE content LIKE ? OR title LIKE ?"
        params = [f"%{query}%", f"%{query}%"]
        if project:
            sql += " AND project = ?"
            params.append(project)
        cur.execute(sql + " LIMIT 50", params)
        for (r,) in cur.fetchall():
            all_ids.add(r)
            fts_scores[r] = 0.5
            
    combined_scores = []
    for cid in all_ids:
        f_s = fts_scores.get(cid, 0.0)
        v_s = vec_scores.get(cid, 0.0)
        if mode == "keyword":
            final_s = f_s
        elif mode == "vector":
            final_s = v_s
        else:
            final_s = 0.4 * f_s + 0.6 * v_s
        combined_scores.append((cid, final_s, f_s, v_s))
        
    combined_scores.sort(key=lambda x: x[1], reverse=True)
    top_results = combined_scores[:top_k]
    
    # Retrieve details
    results = []
    for cid, final_s, f_s, v_s in top_results:
        cur.execute("SELECT project, file_path, obsidian_note_path, chunk_index, title, tags, content, summary FROM chunks WHERE id = ?", (cid,))
        row = cur.fetchone()
        if row:
            results.append({
                "id": cid,
                "project": row[0],
                "file_path": row[1],
                "obsidian_note_path": row[2],
                "chunk_index": row[3],
                "title": row[4],
                "tags": row[5],
                "content": row[6],
                "summary": row[7],
                "score": final_s,
                "bm25_score": f_s,
                "cosine_score": v_s
            })
            
    conn.close()
    return results

def print_search_results(query, results, mode):
    print("\n" + "="*80)
    print(f" 🔍 CS2 AI RAG SEARCH RESULTS")
    print(f" Query: \"{query}\" | Mode: {mode.upper()} | Results Found: {len(results)}")
    print("="*80)
    
    if not results:
        print(" *(No matching chunks found. Try broadening keywords or removing project filter.)*\n")
        return
        
    for i, r in enumerate(results):
        print(f"\nResult #{i+1} [Combined Score: {r['score']:.4f} | BM25: {r['bm25_score']:.4f} | Cosine: {r['cosine_score']:.4f}]")
        print(f"  - Chunk ID:       `{r['id']}`")
        print(f"  - Project:        `{r['project']}`")
        print(f"  - File Path:      `{r['file_path']}`")
        print(f"  - Obsidian Note:  `[[{r['obsidian_note_path']}]]`")
        print(f"  - Tags:           `{r['tags']}`")
        print(f"  - Title:          `{r['title']}`")
        print("  - Excerpt / Content:")
        lines = r['content'].split('\n')
        for l in lines[:12]:
            print(f"      {l[:100]}")
        if len(lines) > 12:
            print(f"      ... (+{len(lines) - 12} more lines)")
        print("-" * 80)
    print("")

def ask_question(question, top_k=4):
    results = search_db(question, top_k=top_k, mode="hybrid")
    print("\n" + "#"*80)
    print(" 🤖 CS2 AI RAG CONTEXT RETRIEVAL & SYNTHESIS")
    print(f" Question: \"{question}\"")
    print("#"*80)
    
    print("\n### 📚 Retrieved Context from Obsidian Vault & Vector DB (`cs2_ai_vectordb.sqlite`):\n")
    for i, r in enumerate(results):
        print(f"**Context Chunk #{i+1}: `{r['project']}` -> `{r['file_path']}` (Score: {r['score']:.4f})**")
        print(f"Obsidian Note: `[[{r['obsidian_note_path']}]]` | Tags: `{r['tags']}`")
        print("```cpp" if r['tags'] and 'cpp' in r['tags'] else "```text")
        print(r['content'][:700])
        if len(r['content']) > 700:
            print("... [truncated]")
        print("```\n")
        
    print("### 🧠 AI Synthesis Summary for Agents & Developers:\n")
    print(f"Based on the **{len(results)}** retrieved chunks from `{DB_PATH}` and `{VAULT_DIR}`:")
    if results:
        proj_counts = {}
        for r in results:
            proj_counts[r['project']] = proj_counts.get(r['project'], 0) + 1
        top_projs = ", ".join([f"`{p}` ({c} chunks)" for p, c in proj_counts.items()])
        print(f"- **Primary Relevant Projects:** {top_projs}")
        print(f"- **Key Files:** {', '.join(set('`' + r['file_path'] + '`' for r in results[:3]))}")
        print(f"- **Guidance:** To inspect the full code around these definitions, open the exact file path or read the linked Obsidian note inside `/home/user/cs2/obsidian_vault/`.")
    else:
        print("- No direct semantic matches found. Consider running `python3 cs2_ai_rag.py list-projects` to see all available frameworks.")
    print("#"*80 + "\n")

def get_chunk(chunk_id):
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute("SELECT id, project, file_path, obsidian_note_path, chunk_index, title, tags, content FROM chunks WHERE id = ?", (chunk_id,))
    row = cur.fetchone()
    conn.close()
    if not row:
        print(f"Error: Chunk ID {chunk_id} not found.")
        return
    print("\n" + "="*80)
    print(f" Chunk ID #{row[0]}: {row[5]}")
    print(f" Project:       {row[1]}")
    print(f" File Path:     {row[2]}")
    print(f" Obsidian Note: {row[3]}")
    print(f" Tags:          {row[6]}")
    print("="*80)
    print("--- Full Chunk Content ---")
    print(row[7])
    print("="*80 + "\n")

def main():
    parser = argparse.ArgumentParser(description="CS2 AI RAG Query Engine")
    subparsers = parser.add_subparsers(dest="command", help="Command to run")
    
    # search
    search_parser = subparsers.add_parser("search", help="Search the vault and vector DB")
    search_parser.add_argument("query", help="Search query string")
    search_parser.add_argument("--top-k", type=int, default=5, help="Number of results to return")
    search_parser.add_argument("--project", type=str, default=None, help="Filter by specific project folder name")
    search_parser.add_argument("--mode", choices=["vector", "keyword", "hybrid"], default="hybrid", help="Search mode")
    
    # ask
    ask_parser = subparsers.add_parser("ask", help="Retrieve RAG context and answer AI question")
    ask_parser.add_argument("question", help="Question to ask AI context engine")
    ask_parser.add_argument("--top-k", type=int, default=4, help="Number of context chunks to retrieve")
    
    # stats
    subparsers.add_parser("stats", help="Show database & vault statistics")
    
    # list-projects
    subparsers.add_parser("list-projects", help="List all indexed subprojects")
    
    # get-chunk
    chunk_parser = subparsers.add_parser("get-chunk", help="Retrieve exact chunk content by ID")
    chunk_parser.add_argument("chunk_id", type=int, help="Chunk ID integer")
    
    args = parser.parse_args()
    
    if args.command == "search":
        results = search_db(args.query, top_k=args.top_k, project=args.project, mode=args.mode)
        print_search_results(args.query, results, args.mode)
    elif args.command == "ask":
        ask_question(args.question, top_k=args.top_k)
    elif args.command == "stats":
        run_stats()
    elif args.command == "list-projects":
        list_projects()
    elif args.command == "get-chunk":
        get_chunk(args.chunk_id)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
