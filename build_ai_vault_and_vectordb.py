#!/usr/bin/env python3
"""
build_ai_vault_and_vectordb.py
==============================
Generates an Obsidian-like Markdown Vault and a self-contained SQLite Hybrid Vector Database (BM25 + Cosine Similarity)
for the CS2 repository (`skiblol346-hue/cs2`) so AI agents and developers can query and learn from the entire codebase.
"""

import os
import sys
import json
import sqlite3
import math
import re
import struct
from datetime import datetime

# Configuration
REPO_ROOT = "/home/user/cs2"
VAULT_DIR = os.path.join(REPO_ROOT, "obsidian_vault")
DB_PATH = os.path.join(REPO_ROOT, "cs2_ai_vectordb.sqlite")

SKIP_DIRS = {".git", "venv", "obsidian_vault", ".download-site"}
TEXT_EXTENSIONS = {
    ".h", ".hpp", ".cpp", ".c", ".cc", ".inc", ".py", ".rs", ".cs", ".zig",
    ".lua", ".ts", ".js", ".md", ".txt", ".json", ".cfg", ".proto",
    ".sln", ".vcxproj", ".filters", ".xml"
}

# Domain keywords for boosted vector representation and tagging
DOMAIN_KEYWORDS = [
    "aimbot", "esp", "chams", "glow", "triggerbot", "bhop", "autostrafe",
    "netvars", "schema", "offsets", "dumper", "memory", "kernel", "driver",
    "hook", "vmt", "minhook", "createmove", "present", "reset", "imgui",
    "directx", "dx11", "gui", "menu", "entity", "player", "pawn", "controller",
    "weapon", "raytrace", "trace", "matrix", "viewmatrix", "worldtoscreen",
    "skeleton", "hitbox", "bone", "radar", "skin", "inventory", "changer",
    "sdk", "interface", "pattern", "signature", "scan", "bypass", "anticheat"
]

def sanitize_filename(name):
    """Sanitize string to make a valid filename or note title."""
    s = re.sub(r'[\/\\\:\*\?\"\<\>\|]', '_', name)
    s = re.sub(r'\s+', '_', s)
    return s.strip('_')

def get_file_tags(filepath, content=""):
    """Extract domain tags based on extension and content keywords."""
    tags = set(["cs2", "code"])
    ext = os.path.splitext(filepath)[1].lower()
    if ext in [".h", ".hpp", ".cpp", ".c", ".cc"]:
        tags.add("cpp")
    elif ext == ".py":
        tags.add("python")
    elif ext == ".rs":
        tags.add("rust")
    elif ext == ".cs":
        tags.add("csharp")
    elif ext == ".zig":
        tags.add("zig")
    elif ext == ".lua":
        tags.add("lua")
    elif ext == ".md":
        tags.add("documentation")
    elif ext == ".json":
        tags.add("json")
    
    # Check domain keywords in filename or first part of content
    lower_text = (os.path.basename(filepath) + " " + content[:2000]).lower()
    for kw in DOMAIN_KEYWORDS:
        if kw in lower_text:
            tags.add(kw)
            
    return sorted(list(tags))

def compute_semantic_vector(text, dim=128):
    """
    Compute a 128-dimensional normalized semantic vector using hashed character/word n-grams
    and domain keyword boosting.
    """
    vector = [0.0] * dim
    if not text:
        return vector
    
    lower_text = text.lower()
    
    # 1. Word tokens and unigrams
    words = re.findall(r'[a-zA-Z0-9_]{3,}', lower_text)
    for w in words:
        h = hash(w) % dim
        vector[h] += 1.0
        
    # 2. Domain keyword boosts (heavy weighting for domain semantics)
    for kw in DOMAIN_KEYWORDS:
        if kw in lower_text:
            count = lower_text.count(kw)
            h = hash(kw) % dim
            vector[h] += 5.0 * count
            
    # 3. Character 4-grams for subword matching
    for i in range(len(lower_text) - 3):
        ngram = lower_text[i:i+4]
        h = hash(ngram) % dim
        vector[h] += 0.2
        
    # L2 Normalization
    norm = math.sqrt(sum(v * v for v in vector))
    if norm > 1e-6:
        vector = [v / norm for v in vector]
    return vector

def vector_to_blob(vector):
    """Pack float list into binary struct BLOB for SQLite."""
    return struct.pack(f'{len(vector)}f', *vector)

def chunk_text(text, max_chars=1800, overlap=50):
    """Split text into overlapping chunks along line boundaries if possible."""
    if not text:
        return []
    lines = text.split('\n')
    chunks = []
    current_chunk = []
    current_len = 0
    
    for line in lines:
        line_len = len(line) + 1
        if current_len + line_len > max_chars and current_chunk:
            chunk_str = '\n'.join(current_chunk).strip()
            if chunk_str:
                chunks.append(chunk_str)
            # Retain overlap lines
            overlap_len = 0
            overlap_chunk = []
            for l in reversed(current_chunk):
                if overlap_len + len(l) + 1 <= overlap:
                    overlap_chunk.insert(0, l)
                    overlap_len += len(l) + 1
                else:
                    break
            current_chunk = overlap_chunk
            current_len = overlap_len
            
        current_chunk.append(line)
        current_len += line_len
        
    if current_chunk:
        chunk_str = '\n'.join(current_chunk).strip()
        if chunk_str:
            chunks.append(chunk_str)
            
    return chunks

def extract_code_highlights(text, ext):
    """Extract class names, struct definitions, functions, or major headers."""
    highlights = []
    lines = text.split('\n')
    for line in lines[:250]:
        line_str = line.strip()
        if ext in [".h", ".hpp", ".cpp", ".c", ".cc"]:
            if re.match(r'^(class|struct|namespace|enum)\s+\w+', line_str):
                highlights.append(line_str.split('{')[0].strip())
            elif re.match(r'^[a-zA-Z0-9_\*:\s]+\s+[a-zA-Z0-9_:]+\([^)]*\)\s*(\{|;|$)', line_str) and not line_str.startswith('return'):
                if len(line_str) < 80 and not line_str.startswith('if') and not line_str.startswith('for'):
                    highlights.append(line_str.split('{')[0].strip())
        elif ext == ".py":
            if line_str.startswith("def ") or line_str.startswith("class "):
                highlights.append(line_str.split(':')[0].strip())
        elif ext in [".rs"]:
            if line_str.startswith("fn ") or line_str.startswith("struct ") or line_str.startswith("pub "):
                highlights.append(line_str.split('{')[0].strip())
    return highlights[:15]

def build_vault_and_db():
    print(f"=== Starting CS2 AI Vault & Vector DB Build ===")
    print(f"Repository Root: {REPO_ROOT}")
    print(f"Obsidian Vault:  {VAULT_DIR}")
    print(f"SQLite DB:       {DB_PATH}\n")
    
    os.makedirs(os.path.join(VAULT_DIR, "Projects"), exist_ok=True)
    os.makedirs(os.path.join(VAULT_DIR, "Code"), exist_ok=True)
    
    # Initialize SQLite Database
    if os.path.exists(DB_PATH):
        os.remove(DB_PATH)
    
    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()
    
    cur.execute("""
        CREATE TABLE metadata (
            key TEXT PRIMARY KEY,
            value TEXT
        );
    """)
    cur.execute("""
        CREATE TABLE chunks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            project TEXT,
            file_path TEXT,
            obsidian_note_path TEXT,
            chunk_index INTEGER,
            title TEXT,
            tags TEXT,
            content TEXT,
            summary TEXT
        );
    """)
    cur.execute("""
        CREATE TABLE embeddings (
            chunk_id INTEGER PRIMARY KEY,
            vector BLOB,
            FOREIGN KEY(chunk_id) REFERENCES chunks(id)
        );
    """)
    cur.execute("""
        CREATE VIRTUAL TABLE chunks_fts USING fts5(
            content,
            title,
            summary,
            tags,
            project,
            file_path,
            content='chunks',
            content_rowid='id'
        );
    """)
    
    # Scan Projects
    projects = []
    for d in os.listdir(REPO_ROOT):
        full_p = os.path.join(REPO_ROOT, d)
        if os.path.isdir(full_p) and d not in SKIP_DIRS and not d.startswith('.'):
            projects.append(d)
    projects.sort()
    
    total_files_scanned = 0
    total_chunks_inserted = 0
    total_notes_created = 0
    
    project_stats = []
    
    for proj in projects:
        proj_dir = os.path.join(REPO_ROOT, proj)
        print(f"Processing project: {proj} ...")
        
        proj_files = []
        ext_counts = {}
        for r, ds, fs in os.walk(proj_dir):
            if '.git' in r or 'venv' in r:
                continue
            for f in fs:
                fpath = os.path.join(r, f)
                proj_files.append(fpath)
                ext = os.path.splitext(f)[1].lower()
                ext_counts[ext] = ext_counts.get(ext, 0) + 1
                
        # Determine project type & tags
        proj_tags = set(["cs2", "project"])
        lower_proj = proj.lower()
        for kw in DOMAIN_KEYWORDS:
            if kw in lower_proj:
                proj_tags.add(kw)
        if ".h" in ext_counts or ".cpp" in ext_counts:
            proj_tags.add("cpp")
        if ".rs" in ext_counts:
            proj_tags.add("rust")
        if ".py" in ext_counts:
            proj_tags.add("python")
            
        proj_tags_list = sorted(list(proj_tags))
        
        # We will create individual Code notes for the first top 25 important files in the project
        # and chunk EVERY valid text file into the Vector DB.
        code_notes_links = []
        files_indexed = 0
        
        for fpath in sorted(proj_files):
            ext = os.path.splitext(fpath)[1].lower()
            if ext not in TEXT_EXTENSIONS and not os.path.basename(fpath).endswith(('.cfg', '.txt', '.sln')):
                continue
                
            try:
                with open(fpath, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
            except Exception:
                continue
                
            if not content.strip():
                continue
                
            rel_path = os.path.relpath(fpath, REPO_ROOT)
            file_tags = get_file_tags(rel_path, content)
            file_tags_str = ", ".join(file_tags)
            highlights = extract_code_highlights(content, ext)
            summary_str = "; ".join(highlights[:5]) if highlights else content[:120].replace('\n', ' ')
            
            # Decide whether to create a dedicated Obsidian note in Code/
            note_filename = f"{sanitize_filename(proj)}_{sanitize_filename(os.path.basename(fpath))}.md"
            note_rel_path = f"Code/{note_filename}"
            full_note_path = os.path.join(VAULT_DIR, note_rel_path)
            
            # Create code note if under limit for this project to keep vault clean
            if len(code_notes_links) < 30 and ext in [".h", ".hpp", ".cpp", ".c", ".rs", ".py", ".cs", ".md", ".json"]:
                code_note_content = [
                    "---",
                    f'title: "{os.path.basename(fpath)}"',
                    f'project: "{proj}"',
                    f'file_path: "{rel_path}"',
                    f'tags: [{", ".join(file_tags)}]',
                    "---",
                    "",
                    f"# File: `{os.path.basename(fpath)}`",
                    "",
                    f"- **Project:** [[Projects/{sanitize_filename(proj)}|{proj}]]",
                    f"- **Path:** `{rel_path}`",
                    f"- **Type:** `{ext if ext else 'text'}`",
                    f"- **Tags:** `{file_tags_str}`",
                    "",
                    "## Highlights / Structure",
                ]
                if highlights:
                    for h in highlights:
                        code_note_content.append(f"- `{h}`")
                else:
                    code_note_content.append("- *(No specific headers/classes extracted)*")
                code_note_content.extend([
                    "",
                    "## Code Excerpt (First ~150 lines)",
                    f"```{ext[1:] if ext else 'text'}",
                    "\n".join(content.split('\n')[:150]),
                    "```",
                    "",
                    f"*Indexed into CS2 AI Vector Database (`cs2_ai_vectordb.sqlite`) for full semantic & keyword retrieval across all chunks.*"
                ])
                
                with open(full_note_path, 'w', encoding='utf-8') as nf:
                    nf.write("\n".join(code_note_content))
                
                code_notes_links.append(f"[[Code/{note_filename[:-3]}|{os.path.basename(fpath)}]]")
                total_notes_created += 1
            else:
                note_rel_path = f"Projects/{sanitize_filename(proj)}.md" # fallback pointer
                
            # Now chunk the file and insert ALL chunks into the SQLite Vector DB!
            chunks = chunk_text(content, max_chars=1800, overlap=50)
            for i, chunk in enumerate(chunks):
                chunk_title = f"{os.path.basename(fpath)} (chunk {i+1}/{len(chunks)})"
                chunk_summary = summary_str if i == 0 else chunk[:100].replace('\n', ' ')
                
                cur.execute("""
                    INSERT INTO chunks (project, file_path, obsidian_note_path, chunk_index, title, tags, content, summary)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                """, (proj, rel_path, note_rel_path, i, chunk_title, file_tags_str, chunk, chunk_summary))
                chunk_id = cur.lastrowid
                
                # Insert into FTS5
                cur.execute("""
                    INSERT INTO chunks_fts (rowid, content, title, summary, tags, project, file_path)
                    VALUES (?, ?, ?, ?, ?, ?, ?)
                """, (chunk_id, chunk, chunk_title, chunk_summary, file_tags_str, proj, rel_path))
                
                # Compute 128D semantic vector and insert into embeddings
                vec = compute_semantic_vector(chunk, dim=128)
                blob = vector_to_blob(vec)
                cur.execute("""
                    INSERT INTO embeddings (chunk_id, vector)
                    VALUES (?, ?)
                """, (chunk_id, blob))
                
                total_chunks_inserted += 1
                
            files_indexed += 1
            total_files_scanned += 1
            
        # Create Project Summary Note in Projects/
        proj_note_filename = f"{sanitize_filename(proj)}.md"
        proj_note_path = os.path.join(VAULT_DIR, "Projects", proj_note_filename)
        
        top_exts = sorted(ext_counts.items(), key=lambda x: x[1], reverse=True)[:5]
        ext_summary_str = ", ".join([f"{k if k else '(no ext)'}: {v}" for k, v in top_exts])
        
        # Generate description based on keywords and contents
        desc_lines = []
        if "osiris" in lower_proj:
            desc = "Open-source internal cheat base for CS2 (originally ported/evolved from Osiris CS:GO). Features comprehensive memory hooks, aimbot, chams, ESP, skin changer, and DirectX/ImGui rendering."
        elif "andromeda" in lower_proj:
            desc = "Andromeda CS2 Base - modern C++ internal framework for CS2 featuring SchemaDumping (`SchemaDump.hpp`), VMT/MinHook interception, interface discovery, and entity iteration."
        elif "aspasia" in lower_proj:
            desc = "Aspasia UI & Cheat Framework - feature-rich Direct3D/ImGui menu, entity manager (`EntityManager/`), Math helpers (`Math/`), HooksManager, and ESP rendering classes for CS2."
        elif "bakaware" in lower_proj:
            desc = "BakaWare4 - C++20/C++23 internal/external cheat engine with dedicated injector, memory manipulation, schema/offset management, and modern CMake build presets."
        elif "dumper" in lower_proj:
            desc = "CS2 Schema & Offset Dumper - extensive tooling in Rust, Zig, C#, C++, and JSON that dumps CS2 client/engine interfaces, netvar offsets, and schema structures (`cs2.json`, `offsets.rs`)."
        elif "fatality" in lower_proj:
            desc = "Fatality CS2 implementation & framework - internal rage/legit cheat engine complete with aimbot math, lag compensation, prediction, anti-aim, and custom GUI framework."
        elif "memesense" in lower_proj or "marusense" in lower_proj or "imgui" in lower_proj:
            desc = "ImGui menu & UI framework reworked for CS2 internal rendering (`memesense-ui`, `Marusense`). Contains custom styling overlays, DirectX hooks, and input handling."
        elif "kernel" in lower_proj or "driver" in lower_proj:
            desc = "Windows Kernel Memory Research & Driver tooling (`windows-kernel-memory-research-main`). Demonstrates kernel memory read/write techniques, CR3 manipulation, and anti-cheat bypass concepts."
        elif "skin" in lower_proj:
            desc = "CS2 Skin Changer & Inventory manipulation framework (`csskin-changer`). Hooks item schema and inventory manager to replace weapon skins, stickers, and models in-game."
        elif "templeware" in lower_proj:
            desc = "TempleWare CS2 - comprehensive internal cheat featuring protobuf parsing (`.proto`), rage/legit bot features, custom menu, and game event hooks."
        elif "ec-main" in lower_proj:
            desc = "EC Framework - dual CS2 / CS:GO project structure with shared library components, memory wrappers, and modular feature architecture."
        else:
            desc = f"CS2 internal/external cheat project (`{proj}`) containing {len(proj_files)} files (`{ext_summary_str}`). Includes memory/interface wrappers, hooks, and CS2 feature modules."
            
        proj_note_lines = [
            "---",
            f'title: "{proj}"',
            f'type: "project_overview"',
            f'tags: [{", ".join(proj_tags_list)}]',
            f'total_files: {len(proj_files)}',
            f'indexed_text_files: {files_indexed}',
            "---",
            "",
            f"# Project: `{proj}`",
            "",
            f"- **Vault Master Index:** [[../_00_Master_Vault_Index|_00_Master_Vault_Index]]",
            f"- **Total Files:** `{len(proj_files)}` (`{ext_summary_str}`)",
            f"- **Indexed Text/Code Files:** `{files_indexed}`",
            f"- **Domain Tags:** `{', '.join(proj_tags_list)}`",
            "",
            "## Description & Architecture Overview",
            f"{desc}",
            "",
            "## Key File Summary Table",
            "| File Extension | Count |",
            "|---|---|"
        ]
        for k, v in top_exts:
            proj_note_lines.append(f"| `{k if k else '(no ext)'}` | {v} |")
            
        proj_note_lines.extend([
            "",
            "## Highlighted Code Notes in Vault",
            "The following core code files have dedicated Markdown notes in the `Code/` directory of this vault. **All** files in this project are also fully chunked and indexed into the vector database (`cs2_ai_vectordb.sqlite`):",
            ""
        ])
        if code_notes_links:
            for cnl in code_notes_links[:25]:
                proj_note_lines.append(f"- {cnl}")
        else:
            proj_note_lines.append("- *(No specific code notes generated; query via Vector DB)*")
            
        proj_note_lines.extend([
            "",
            "## Semantic & RAG Query Guidance",
            f"To query functions, offsets, structs, or hooks within `{proj}`, run the AI RAG query tool:",
            f"```bash",
            f"python3 cs2_ai_rag.py search \"<your search query>\" --project \"{proj}\"",
            f"```"
        ])
        
        with open(proj_note_path, 'w', encoding='utf-8') as pf:
            pf.write("\n".join(proj_note_lines))
            
        total_notes_created += 1
        project_stats.append({
            "name": proj,
            "files": len(proj_files),
            "indexed": files_indexed,
            "exts": ext_summary_str,
            "desc": desc,
            "note_path": f"Projects/{proj_note_filename}"
        })
        
    # Create Master Index Note
    master_index_path = os.path.join(VAULT_DIR, "_00_Master_Vault_Index.md")
    master_lines = [
        "---",
        'title: "CS2 AI Master Vault Index"',
        'repository: "skiblol346-hue/cs2"',
        f'generated_date: "{datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S UTC")}"',
        'tags: [cs2, master_index, ai_vault, vector_db, rag]',
        f'total_projects: {len(projects)}',
        f'total_notes: {total_notes_created + 1}',
        f'total_chunks: {total_chunks_inserted}',
        "---",
        "",
        "# 🌐 CS2 AI Master Vault Index (`skiblol346-hue/cs2`)",
        "",
        "Welcome to the **Obsidian-like AI Knowledge Base & Hybrid Vector Database** for the `skiblol346-hue/cs2` repository.",
        "This vault formats all 51 internal/external cheatbases, UI frameworks, kernel memory research tools, and offset dumpers into structured Markdown linked via `[[Wikilinks]]`, while `cs2_ai_vectordb.sqlite` stores hybrid 128-dimensional semantic embeddings and full-text BM25 indexes for instant RAG queries.",
        "",
        "## 📊 Repository & Vault Statistics",
        f"- **Total Projects / Root Folders:** `{len(projects)}`",
        f"- **Total Scanned Text/Code Files:** `{total_files_scanned}`",
        f"- **Total Obsidian Notes Generated:** `{total_notes_created + 1}` (`Projects/` + `Code/`)",
        f"- **Total Vector & FTS Chunks Indexed:** `{total_chunks_inserted}`",
        f"- **Vector DB Engine:** `cs2_ai_vectordb.sqlite` (128D Hashed n-gram + Domain Keyword Cosine Embeddings + FTS5 BM25 Hybrid Search)",
        f"- **AI Query Tool:** `python3 cs2_ai_rag.py search \"<query>\"`",
        "",
        "## 🧭 Master Project Directory Table",
        "Below is the complete catalogue of all subprojects in this repository. Click any wikilink (`[[Projects/...]]`) to navigate to that project's detailed note, architecture, and file highlights:",
        "",
        "| Project Name | Total Files | Indexed Files | Top Extensions | Overview / Description |",
        "|---|---|---|---|---|"
    ]
    
    for ps in project_stats:
        clean_name = sanitize_filename(ps['name'])
        link = f"[[Projects/{clean_name}|{ps['name']}]]"
        master_lines.append(f"| {link} | {ps['files']} | {ps['indexed']} | `{ps['exts']}` | {ps['desc'][:110]}... |")
        
    master_lines.extend([
        "",
        "## 🛠️ How AI Agents & Developers Use This Vault",
        "1. **Obsidian Vault Navigation (`obsidian_vault/`)**:",
        "   - Open `_00_Master_Vault_Index.md` (this file) to browse top-level projects.",
        "   - Navigate to any project note in `Projects/*.md` to inspect folder architecture, tags, and key code links.",
        "   - Check `Code/*.md` for specific source code structure highlights and excerpts.",
        "2. **Hybrid RAG Vector & Keyword Querying (`cs2_ai_rag.py`)**:",
        "   - Run keyword search (`FTS5 BM25`): `python3 cs2_ai_rag.py search \"dwLocalPlayerPawn\" --mode keyword`",
        "   - Run semantic vector search (`Cosine Similarity`): `python3 cs2_ai_rag.py search \"how kernel memory read works\" --mode vector`",
        "   - Run hybrid search (`Combined 50/50 score`): `python3 cs2_ai_rag.py search \"aimbot bone smoothing calculation\" --mode hybrid --top-k 5`",
        "   - Retrieve any chunk or note directly: `python3 cs2_ai_rag.py get-chunk <chunk_id>` or `python3 cs2_ai_rag.py ask \"What is Osiris?\"`",
        ""
    ])
    
    with open(master_index_path, 'w', encoding='utf-8') as mf:
        mf.write("\n".join(master_lines))
        
    # Save Metadata to SQLite
    cur.execute("INSERT OR REPLACE INTO metadata VALUES ('vault_build_date', ?)", (datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S UTC"),))
    cur.execute("INSERT OR REPLACE INTO metadata VALUES ('total_projects', ?)", (str(len(projects)),))
    cur.execute("INSERT OR REPLACE INTO metadata VALUES ('total_files_scanned', ?)", (str(total_files_scanned),))
    cur.execute("INSERT OR REPLACE INTO metadata VALUES ('total_notes_created', ?)", (str(total_notes_created + 1),))
    cur.execute("INSERT OR REPLACE INTO metadata VALUES ('total_chunks_inserted', ?)", (str(total_chunks_inserted),))
    cur.execute("INSERT OR REPLACE INTO metadata VALUES ('embedding_dim', '128')", )
    
    conn.commit()
    conn.close()
    
    print(f"\n=== Build Complete! ===")
    print(f"Total Projects Indexed: {len(projects)}")
    print(f"Total Text/Code Files:  {total_files_scanned}")
    print(f"Total Obsidian Notes:   {total_notes_created + 1}")
    print(f"Total Vector DB Chunks: {total_chunks_inserted}")
    print(f"Vault Master Index:     {master_index_path}")
    print(f"Vector Database File:   {DB_PATH}")

if __name__ == "__main__":
    build_vault_and_db()
