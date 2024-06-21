import _ from 'lodash';

import Request from '@/lib/request/Request.ts';
import Response from '@/lib/response/Response.ts';
import core from '../controllers/core.ts';
import chat from '@/api/controllers/chat.ts';
import logger from '@/lib/logger.ts';
import audio from '@/api/controllers/audio.ts';
import fs from "fs-extra";
import path from 'path';

export default {

  prefix: '/v1/chat',

  post: {

      '/completions': async (request: Request) => {
          request
              .validate('body.conversation_id', v => _.isUndefined(v) || _.isString(v))
              .validate('body.messages', _.isArray)
              .validate('headers.authorization', _.isString)
          // token切分
          const tokens = core.tokenSplit(request.headers.authorization);
          // 随机挑选一个token
          const token = _.sample(tokens);
          const { model, conversation_id: convId, messages, stream } = request.body;
          if (stream) {
              const stream = await chat.createCompletionStream(model, messages, token, convId);
              return new Response(stream, {
                  type: "text/event-stream"
              });
          }
          else
              return await chat.createCompletion(model, messages, token, convId);
      },
      
      "/phone_msg": async (request: Request) => {
      request
        .validate("body.model", _.isString)
        .validate("body.response_format", v => _.isUndefined(v) || _.isString(v))
        .validate("headers.authorization", _.isString);
      // token切分
      const tokens = core.tokenSplit(request.headers.authorization);
      // 随机挑选一个token
      const token = _.sample(tokens);
      // logger.info("tokens:", tokens);
      // logger.info("token:", token);
      if(!request.files['file'] && !request.body["file"])
        throw new Error('File field is not set');
      let tmpFilePath;
      if(request.files['file']) {
        const file = request.files['file'];
        if(!['audio/mp3', 'audio/mpeg', 'audio/x-wav', 'audio/wave', 'audio/mp4a-latm', 'audio/flac', 'audio/ogg', 'audio/webm'].includes(file.mimetype))
          throw new Error(`File MIME type ${file.mimetype} is unsupported`);
        tmpFilePath = file.filepath;
        logger.info("tmpFilePath：",tmpFilePath);
      }
      else
        throw new Error('File field is not set');
      const { model, response_format: responseFormat = 'text' } = request.body;
      if(responseFormat == "text")
      {
        const bot_filepath = await audio.createPhomeMsg(model, tmpFilePath, token, "text");
        if(bot_filepath === null)
          throw new Error("BOT voice message is empty");
        else
          // 返回文件路径
          return new Response(`http://192.168.0.5:8000/v1/chat/voice_file/${path.basename(bot_filepath)}`);
      }else{
        // 直接返回二进制数据
        const binary = await audio.createPhomeMsg(model, tmpFilePath, token, "voice");
        if (binary === null)
          throw new Error("BOT voice message is empty");
        else
          return new Response(binary, {
            type: "audio/wav",
            headers: {
              "Content-Disposition": "attachment; filename=phone_msg.wav"
            },
          });
      }
    },
      
    "/phone_voice": async (request: Request) => {
      request
        .validate("headers.authorization", _.isString);

      // token切分
      const tokens = core.tokenSplit(request.headers.authorization);
      // 随机挑选一个token
      const token = _.sample(tokens);

      if(request.headers['content-type'] === 'application/octet-stream' || request.headers['content-type'] === 'audio/wave') {
        // 等待数据到达
        await new Promise(resolve => setTimeout(resolve, 1000));
        logger.info("request.body:", request.binary.length);
        // 将二进制数据写入文件
        const tmpFilePath = `./voice_temp/${Date.now()}_record.wav`;
        await fs.writeFile(tmpFilePath, request.binary);
        const { model, response_format: responseFormat = 'text' } = request.query;
        if(responseFormat == "text")
        {
          const bot_filepath = await audio.createPhomeMsg(model, tmpFilePath, token, "text");
          if(bot_filepath === null)
            throw new Error("BOT voice message is empty");
          else
            // 返回文件路径
            return new Response(`http://192.168.0.5:8000/v1/chat/voice_file/${path.basename(bot_filepath)}`);
        }else{
          // 直接返回二进制数据
          const binary = await audio.createPhomeMsg(model, tmpFilePath, token, "voice");
          if (binary === null)
            throw new Error("BOT voice message is empty");
          else
            return new Response(binary, {
              type: "audio/wav",
              headers: {
                "Content-Disposition": "attachment; filename=phone_msg.wav"
              },
            });
        }
      }

      
    },
  },
  get: {
    //api: voice_file/xxxx.wav 返回对应的wav文件

    "/voice_file/:filename": async (request: Request, res: Response) => {
      const filename = request.params.filename;
      const filepath = `./voice_temp/${filename}`;
      logger.info(`${filename}`);
      logger.info(`${filepath}`);
      // 检查文件是否存在
      const buffer = await fs.readFile(filepath);
      // logger.info("buffer:", buffer);
      return new Response(buffer, {
        type: "audio/wav",
        headers: {
          "Content-Disposition": `attachment; filename=phone_msg.wav`,
        },
      });
    },
  },
};