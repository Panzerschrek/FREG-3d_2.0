" Vim syntax file
" Language: freg maps
" Maintainer: Alexander Kromm
" Latest Revision: 2014-04-17

if exists("b:current_syntax")
    finish
endif

" Keywords
syn match fregMapElementWater  '\~'
syn match fregMapElementPlain  '\.'
syn match fregMapElementHill   '+'
syn match fregMapElementForest '%'
syn match fregMapElementMount  '\^'
syn match fregMapElementNull   '#'
syn match fregMapElementDesert ':'
syn match fregMapElementLava   'l'
syn match fregMapElementAcid   'a'
syn match fregMapElementCrater 'c'
syn match fregMapElementDeadForest 'f'
syn match fregMapElementDeadHill   '*'

let b:current_syntax = "fregMap"

hi fregMapElementWater  ctermfg=Cyan    ctermbg=Blue
hi fregMapElementPlain  ctermfg=Black   ctermbg=Green
hi fregMapElementHill   ctermfg=White   ctermbg=Green
hi fregMapElementMount  ctermfg=Black   ctermbg=White
hi fregMapElementForest ctermfg=Yellow  ctermbg=Green
hi fregMapElementNull   ctermfg=Magenta ctermbg=Black
hi fregMapElementDesert ctermfg=White   ctermbg=Yellow
hi fregMapElementLava   ctermfg=Red     ctermbg=Yellow
hi fregMapElementAcid   ctermfg=Green   ctermbg=LightGreen
hi fregMapElementCrater ctermfg=White   ctermbg=Grey
hi fregMapElementDeadForest ctermfg=Yellow   ctermbg=Black
hi fregMapElementDeadHill   ctermfg=DarkGrey ctermbg=White
